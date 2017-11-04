#version 330 

#define MAX_NB_LIGHTS 25
#define PI 3.14159265358979323846264338

struct Material
{    
  vec3 _albedo;
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
uniform sampler2D uGbufferPosition;
uniform sampler2D uGbufferNormal;
uniform sampler2D uGbufferAlbedo;
uniform sampler2D uGbufferRougnessMetalnessAO;

uniform int   uLightCount;
uniform vec3  uLightPos[ MAX_NB_LIGHTS ];
uniform vec3  uLightColor[ MAX_NB_LIGHTS ];
uniform float uLightIntensity[ MAX_NB_LIGHTS ];

uniform vec3 uViewPos;


// Fragment inputs from vertex shader 
// ----------------------------------
in vec2 oUV;


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************


// PBR lighting functions
// ----------------------
float DistributionGGX( vec3 iNormal,
                       vec3 iHalfway,
                       float iRoughness )
{
  float a = iRoughness * iRoughness;
  float a2 = a * a;
  float NdotH = max( dot( iNormal, iHalfway ), 0.0);
  float NdotH2 = NdotH * NdotH;

  float nom  = a2;
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

float GeometrySmith( vec3 iNormal,
                     vec3 iViewDir,
                     vec3 iLightDir,
                     float iRoughness )
{
  float NdotV = max( dot ( iNormal, iViewDir ), 0.0 );
  float NdotL = max( dot( iNormal, iLightDir ), 0.0 );
  float ggx2 = GeometrySchlickGGX( NdotV, iRoughness );
  float ggx1 = GeometrySchlickGGX( NdotL, iRoughness );

  return ggx1 * ggx2;
}

vec3 FresnelSchlick( float iCosTheta,
                     vec3 iF0 )
{
  return iF0 + ( 1.0 - iF0 ) * pow( 1.0 - iCosTheta, 5.0 );
}

vec3 FresnelSchlickRoughness( float iCosTheta,
                              vec3 iF0,
                              float iRoughness )
{
  return iF0 + ( max( vec3( 1.0 - iRoughness), iF0 ) - iF0 ) * pow( 1.0 - iCosTheta, 5.0 );
}

vec3 ReflectanceEquationCalculation( vec3     iFragPos,
                                     vec3     iViewDir,
                                     vec3     iNormal,
                                     vec3     iF0,
                                     float    iMaxNormalDotViewDir,
                                     Material iMaterial )
{ 

  // Pre calculation optimisation
  // ----------------------------
  float IV_max_dot_N_V = 4 * iMaxNormalDotViewDir; 
  vec3 albedo_by_PI    = iMaterial._albedo / PI;


  // Compute equation to each scene light
  // ------------------------------------
  vec3 Lo = vec3( 0.0 );
  for( int i = 0; i < uLightCount; i++ ) 
  {

    // Calculate per-light radiance
    // ----------------------------
    
    // Get light -> frag distance
    float distance = length( uLightPos[ i ] - iFragPos );

    // Get attenuation value
    float attenuation = 1.0 / ( distance * distance );
    
    // Get light radiance value
    vec3 light_radiance = ( uLightColor[ i ] * uLightIntensity[ i ] ) * attenuation;


    // Cook-Torrance BRDF ( specular )
    // -------------------------------
    
    // Get light direction
    vec3 light_dir = normalize( uLightPos[ i ] - iFragPos );
   
    // Get halfway vector
    vec3 halfway = normalize( iViewDir + light_dir );

    // Specular BRDF calculation
    float NDF = DistributionGGX( iNormal, halfway, iMaterial._roughness );   
    vec3 F    = FresnelSchlick( max( dot( halfway, iViewDir ), 0.0 ), iF0 );
    float G   = GeometrySmith( iNormal, iViewDir, light_dir, iMaterial._roughness );      
    vec3 nominator      = NDF * F * G; 
    float denominator   = ( IV_max_dot_N_V * max( dot( iNormal, light_dir ) , 0.0 ) ) + 0.001; // 0.001 to prevent divide by zero.
    vec3 light_specular = nominator / denominator;
        

    // Get kS value
    // ------------
    vec3 kS = F;


    // Get kD, with energy conservation
    // --------------------------------
    vec3 kD = vec3( 1.0 ) - kS;
    
    // Decrease diffuse by the metalness, pure metal have no diffuse light
    kD *= 1.0 - iMaterial._metalness;   


    // Get NdotL value
    // ---------------
    float normal_dot_light_dir = max( dot( iNormal, light_dir ), 0.0 );        


    // Final light influence
    // ---------------------
    Lo += ( ( kD * ( albedo_by_PI ) ) + light_specular ) * light_radiance * normal_dot_light_dir;  // already multiplied the specular by the Fresnel ( kS )
    
  }   

  return Lo;
}

vec3 PBRLightingCalculation()
{
  // Get G-buffer data
  vec3 normal = texture( uGbufferNormal, oUV ).rgb;
  if( normal == vec3( 0.0 ) )
  {
    discard;
  }
  vec3 frag_pos = texture( uGbufferPosition, oUV ).rgb;
  vec3 roughness_metalness_AO = texture( uGbufferRougnessMetalnessAO, oUV ).rgb;

  // Get material inputs data
  Material material;
  material._albedo    = texture( uGbufferAlbedo, oUV ).rgb;
  material._roughness = roughness_metalness_AO.r;
  material._metalness = roughness_metalness_AO.g;
  material._ao        = roughness_metalness_AO.b;

  // Get TBN matrix
  /*mat3 TBN;
  TBN[ 0 ] = oTBN[ 0 ];
  TBN[ 1 ] = oTBN[ 1 ];
  TBN[ 2 ] = oTBN[ 2 ];*/

  // Get camera view direction vector
  vec3 view_dir = normalize( uViewPos - frag_pos );

  // Get surface base reflectivity value
  vec3 F0 = vec3( 0.04 ); 
  F0 = mix( F0, material._albedo, material._metalness );  

  // Pre calcul max_dot_N_V
  float max_dot_N_V = max( dot( normal, view_dir ), 0.0 );

  
  // BRDF calculation part
  // ---------------------
  
  // Compute reflectance equation calculation to each scene light
  vec3 lights_reflectance = ReflectanceEquationCalculation( frag_pos,
                                                            view_dir,
                                                            normal,
                                                            F0,
                                                            max_dot_N_V,
                                                            material );


  // IBL calculation part
  // --------------------
   
  // Compute indirect irradiance
  /*vec3 diffuse_IBL = IndirectIrradianceCalculation( max_dot_N_V,
                                                    material,
                                                    F0,
                                                    iNormal,
                                                    TBN );*/
  

  // Return fragment final PBR lighting 
  // ----------------------------------
  return /*diffuse_IBL +*/ lights_reflectance;
}


// Main
// ----
void main()
{ 
  // PBR lighting calculation 
  vec3 PBR_lighting_result = PBRLightingCalculation(); 

  // Main out color
  FragColor = vec4( PBR_lighting_result, 1.0 );
  //FragColor = vec4( vec3( 1.0, 0.0, 0.0 ), 1.0 );


  // Second out color => draw only brightest fragments
  vec3 bright_color = vec3( 0.0, 0.0, 1.0 );
  /*if( uBloom )
  {
    float brightness = dot( PBR_lighting_result, vec3( 0.2126, 0.7152, 0.0722 ) );
    if( brightness > uBloomBrightness )
    {
      bright_color = PBR_lighting_result;
    }
  }*/
  FragColorBrightness = vec4( bright_color, 1.0 );
}
