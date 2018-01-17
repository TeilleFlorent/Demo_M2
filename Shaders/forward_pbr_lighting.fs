#version 330 

#define MAX_NB_LIGHTS 25
#define PI 3.14159265358979323846264338

struct Material
{    
  vec3  _albedo;
  float _metalness;
  float _roughness;
  float _ao; 
};


//******************************************************************************
//**********  Fragment shader inputs/ouputs  ***********************************
//******************************************************************************


// Fragment color output(s)
// ------------------------
layout ( location = 0 ) out vec4 FragColor;
layout ( location = 1 ) out vec4 FragColorBrightness;


// Fragment input uniforms
// -----------------------
uniform int   uLightCount;
uniform vec3  uLightPos[ MAX_NB_LIGHTS ];
uniform vec3  uLightColor[ MAX_NB_LIGHTS ];
uniform float uLightIntensity[ MAX_NB_LIGHTS ];

uniform vec3 uViewPos;

uniform float uBloom;
uniform float uBloomBrightness;

uniform float uMaxMipLevel;

uniform float uAlpha;
uniform float uID;

uniform sampler2D   uTextureDiffuse1; 
uniform sampler2D   uTextureNormal1; 
uniform sampler2D   uTextureHeight1; 
uniform sampler2D   uTextureAO1; 
uniform sampler2D   uTextureRoughness1; 
uniform sampler2D   uTextureMetalness1; 
uniform sampler2D   uTextureSpecular1;

uniform samplerCube uIrradianceCubeMap;
uniform samplerCube uPreFilterCubeMap;
uniform sampler2D   uPreBrdfLUT;


// Fragment inputs from vertex shader 
// ----------------------------------
in vec2 oUV;
in vec3 oFragPos;
in vec3 oTBN[ 3 ];


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************


// Normal mapping function
vec3 NormalMappingCalculation( vec2 iUV )                                                                     
{      
  vec3 res_normal;
      
  res_normal = texture( uTextureNormal1, iUV ).rgb;
  res_normal = normalize( res_normal * 2.0 - 1.0 );   

  return res_normal;
}


// Cook torrance D function
float DistributionGGX( vec3  iNormal,
                       vec3  iHalfway,
                       float iRoughness )
{
  float a  = iRoughness * iRoughness;
  float a2 = a * a;
  float NdotH  = max( dot( iNormal, iHalfway ), 0.0);
  float NdotH2 = NdotH * NdotH;

  float nom   = a2;
  float denom = ( NdotH2 * ( a2 - 1.0 ) + 1.0 );
  denom = PI * denom * denom;

  return nom / denom;
}

float GeometrySchlickGGX( float NdotV,
                          float iRoughness )
{
  float r = ( iRoughness + 1.0 );
  float k = ( r * r ) / 8.0;

  float nom   = NdotV;
  float denom = NdotV * ( 1.0 - k ) + k;

  return nom / denom;
}

// Cook torrance G function
float GeometrySmith( vec3 iNormal,
                     vec3 iViewDir,
                     vec3 iLightDir,
                     float iRoughness )
{
  float NdotV = max( dot( iNormal, iViewDir ), 0.0 );
  float NdotL = max( dot( iNormal, iLightDir ), 0.0 );
  float ggx2  = GeometrySchlickGGX( NdotV, iRoughness );
  float ggx1  = GeometrySchlickGGX( NdotL, iRoughness );

  return ggx1 * ggx2;
}

// Cook torrance F function
vec3 FresnelSchlick( float iCosTheta,
                     vec3 iF0 )
{
  return iF0 + ( 1.0 - iF0 ) * pow( 1.0 - iCosTheta, 5.0 );
}

// Cook torrance F function taking account of the surface's roughness
vec3 FresnelSchlickRoughness( float iCosTheta,
                              vec3 iF0,
                              float iRoughness )
{
  return iF0 + ( max( vec3( 1.0 - iRoughness), iF0 ) - iF0 ) * pow( 1.0 - iCosTheta, 5.0 );
}   

// Cook torrance for point light function
vec3 PointLightReflectance( vec2     iUV,
                            vec3     iViewDir,
                            vec3     iNormal,
                            vec3     iF0,
                            float    iNormalDotViewDir,
                            Material iMaterial )
{ 

  // Pre calculation optimisation
  // ----------------------------
  float IV_N_dot_V  = 4 * iNormalDotViewDir; 
  vec3 albedo_by_PI = iMaterial._albedo / PI;


  // Compute equation to each scene light
  // ------------------------------------
  vec3 Lo = vec3( 0.0 );
  for( int i = 0; i < uLightCount; i++ ) 
  {

    // Calculate per-light radiance
    // ----------------------------
    
    // Get light direction
    vec3 light_dir = uLightPos[ i ] - oFragPos;

    // Get light -> frag distance
    float distance = length( light_dir );

    // Get attenuation value
    float attenuation = 1.0 / ( distance * distance );
    
    // Get light radiance value
    vec3 light_radiance = ( uLightColor[ i ] * uLightIntensity[ i ] ) * attenuation;


    // Cook-Torrance BRDF ( specular )
    // -------------------------------
    
    // Get light direction
    light_dir = normalize( light_dir );
   
    // Get halfway vector
    vec3 halfway = normalize( iViewDir + light_dir );

    // Specular BRDF calculation
    float NDF = DistributionGGX( iNormal, halfway, iMaterial._roughness );   
    vec3 F    = FresnelSchlick( max( dot( halfway, iViewDir ), 0.0 ), iF0 );
    float G   = GeometrySmith( iNormal, iViewDir, light_dir, iMaterial._roughness );      
    vec3 nominator      = NDF * F * G; 
    float denominator   = ( IV_N_dot_V * max( dot( iNormal, light_dir ) , 0.0 ) ) + 0.001; // 0.001 to prevent divide by zero.
    vec3 light_specular = nominator / denominator;
        

    // Get kS value
    // ------------
    vec3 kS = F;


    // Get kD, with energy conservation
    // --------------------------------
    vec3 kD = vec3( 1.0 ) - kS;
    
    // Decrease diffuse by the metalness, pure metal have no diffuse light
    kD *= 1.0 - iMaterial._metalness;   


    // Get Normal dot LightDir value
    // -----------------------------
    float normal_dot_light_dir = clamp( dot( iNormal, light_dir ), 0.0, 1.0 );        


    // Final light influence
    // ---------------------
    Lo += ( ( kD * ( albedo_by_PI ) ) + light_specular ) * light_radiance * normal_dot_light_dir;  // already multiplied the specular by the Fresnel ( kS )
    
  }   

  return Lo;
} 

// PBR IBL calculation
vec3 IBLAmbientReflectance( float iNormalDotViewDir,
                            Material iMaterial,
                            vec3 iF0,
                            vec3 iNormal,
                            vec3 iViewDir )
{

  // Diffuse irradiance calculation
  // ------------------------------

  // Get kS taking account the roughness
  vec3 kS = FresnelSchlickRoughness( iNormalDotViewDir, iF0, iMaterial._roughness );
  
  // Get kD with energy conservation
  vec3 kD = 1.0 - kS;

  // Decrease diffuse by the metalness, pure metal have no diffuse light
  kD *= ( 1.0 - iMaterial._metalness );   

  // Sample pre compute irradiance cubemap
  vec3 irradiance = texture( uIrradianceCubeMap, iNormal ).rgb;

  vec3 diffuse = ( irradiance * iMaterial._albedo ) * kD;
  

  // Specular reflectance calculation
  // --------------------------------
  
  // Get reflection vector
  vec3 reflect = reflect( -iViewDir, iNormal ); 

  // Sample specular pre filtered color with a mip level corresponding to the given roughness
  vec3 prefiltered_color = textureLod( uPreFilterCubeMap, reflect,  iMaterial._roughness * uMaxMipLevel ).rgb; 

  // Sample the BRDF look up texture with the given angle and roughness to get the corresponding scale and bias to F0
  vec2 brdf = texture( uPreBrdfLUT, vec2( max( dot( iNormal, iViewDir ), 0.0 ), iMaterial._roughness) ).rg;

  // Compute IBL specular color
  vec3 specular = prefiltered_color * ( ( kS * brdf.x ) + brdf.y );


  // Compute final ambient IBL lighting
  // ----------------------------------
  vec3 ambient = ( diffuse + specular ) * iMaterial._ao; 

  return ambient;
}

// Complete PBR lighting calculation
vec3 PBRLightingCalculation( vec3 iNormal,
                             vec3 iViewDir,  
                             vec2 iUV )
{
  // Get material inputs data
  Material material;
  material._albedo    = pow( texture( uTextureDiffuse1, iUV ).rgb, vec3( 2.2 ) );
  //material._albedo    = pow( vec3( 1.0 ), vec3( 2.2 ) );
  material._metalness = texture( uTextureMetalness1, iUV ).r;
  material._roughness = texture( uTextureRoughness1, iUV ).r;
  material._ao        = texture( uTextureAO1, iUV ).r;

  // Get surface base reflectivity value
  vec3 F0 = vec3( 0.04 ); 
  F0 = mix( F0, material._albedo, material._metalness );  

  // Pre calcul N dot V
  float N_dot_V = max( dot( iNormal, iViewDir ), 0.0 );

  
  // BRDF calculation part
  // ---------------------
  
  // Compute reflectance equation calculation to each scene light
  vec3 point_lights_reflectance = PointLightReflectance( iUV,
                                                         iViewDir,
                                                         iNormal,
                                                         F0,
                                                         N_dot_V,
                                                         material );


  // IBL calculation part
  // --------------------
   
  // Compute indirect irradiance
  vec3 ambient_reflectance = IBLAmbientReflectance( N_dot_V,
                                                    material,
                                                    F0,
                                                    iNormal,
                                                    iViewDir );
        

  // Return fragment final PBR lighting 
  // ----------------------------------
  return ambient_reflectance + point_lights_reflectance;
}


// Main
// ----
void main()
{
  vec3 PBR_lighting_result;
  vec2 final_UV;
  vec3 view_dir;

  // Get TBN matrix
  mat3 TBN;
  TBN[ 0 ] = oTBN[ 0 ];
  TBN[ 1 ] = oTBN[ 1 ];
  TBN[ 2 ] = oTBN[ 2 ];

  // Get view direction
  view_dir = normalize( uViewPos - oFragPos );

  // Normal mapping calculation
  vec3 norm = NormalMappingCalculation( oUV ) * TBN;

  // PBR lighting calculation 
  PBR_lighting_result = PBRLightingCalculation( norm,
                                                view_dir,  
                                                oUV );

  // Main out color
  FragColor = vec4( PBR_lighting_result, uAlpha );

  // Second out color => draw only brightest fragments
  vec3 bright_color = vec3( 0.0, 0.0, 0.0 );
  if( uBloom == 1.0 )
  {
    float brightness = dot( PBR_lighting_result, vec3( 0.2126, 0.7152, 0.0722 ) );
    if( brightness > uBloomBrightness )
    {
      bright_color = PBR_lighting_result;
    }
  }
  FragColorBrightness = vec4( bright_color, uAlpha );
}
