#version 410

#define MAX_NB_LIGHTS 25
#define PI 3.14159265358979323846264338
#define ZERO 0.00390625

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

// Point lights uniforms
uniform int   uLightCount;
uniform vec3  uLightPos[ MAX_NB_LIGHTS ];
uniform vec3  uLightColor[ MAX_NB_LIGHTS ];
uniform float uLightIntensity[ MAX_NB_LIGHTS ];

// View uniforms
uniform vec3 uViewPos;

// Bloom uniforms
uniform bool  uBloom;
uniform float uBloomBrightness;

// IBL uniforms
uniform bool  uIBL;
uniform float uMaxMipLevel;
uniform bool  uParallaxCubemap;
uniform vec3  uCubemapPos;
uniform bool  uIsWall;

// Opacity uniforms
uniform bool  uOpacityMap;
uniform float uOpacityDiscard;
uniform float uAlpha;

// Normal mapping uniforms
uniform bool uNormalMap;

// Shadow uniforms
uniform bool  uReceivShadow;
uniform float uShadowFar;
uniform int   uLightSourceIt;
uniform float uShadowBias;
uniform float uShadowDarkness;

// Emissive uniform(s)
uniform bool  uEmissive;
uniform float uEmissiveFactor;

// Scene object ID uniform
uniform float uID;

// Textures uniforms
uniform sampler2D   uTextureAlbedo1; 
uniform sampler2D   uTextureNormal1; 
uniform sampler2D   uTextureHeight1; 
uniform sampler2D   uTextureAO1; 
uniform sampler2D   uTextureRoughness1; 
uniform sampler2D   uTextureMetalness1; 
uniform sampler2D   uTextureOpacity1; 
uniform sampler2D   uTextureEmissive1; 

uniform samplerCube uIrradianceCubeMap;
uniform samplerCube uPreFilterCubeMap;
uniform sampler2D   uPreBrdfLUT;

uniform samplerCube uDepthCubeMap;


// Fragment inputs from vertex shader 
// ----------------------------------
in vec3 oFragPos;
in vec3 oNormal;
in vec2 oUV;
in vec3 oTBN[ 3 ];


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************


// Normal mapping function
vec3 NormalMappingCalculation( vec2 iUV )                                                                     
{      
  vec3 res_normal;

  // Get TBN matrix
  mat3 TBN;
  TBN[ 0 ] = oTBN[ 0 ];
  TBN[ 1 ] = oTBN[ 1 ];
  TBN[ 2 ] = oTBN[ 2 ];
      
  res_normal = texture( uTextureNormal1, iUV ).rgb;
  res_normal = normalize( res_normal * 2.0 - 1.0 );   

  return ( res_normal * TBN );
}

// PCF preset offset directions use to sample the depth cubemap
vec3 PCF_offset_directions[ 20 ] = vec3[]
(
  vec3( 1, 1,  1 ), vec3(  1, -1,  1 ), vec3( -1, -1,  1 ), vec3( -1, 1,  1 ), 
  vec3( 1, 1, -1 ), vec3(  1, -1, -1 ), vec3( -1, -1, -1 ), vec3( -1, 1, -1 ),
  vec3( 1, 1,  0 ), vec3(  1, -1,  0 ), vec3( -1, -1,  0 ), vec3( -1, 1,  0 ),
  vec3( 1, 0,  1 ), vec3( -1,  0,  1 ), vec3(  1,  0, -1 ), vec3( -1, 0, -1 ),
  vec3( 0, 1,  1 ), vec3(  0, -1,  1 ), vec3(  0, -1, -1 ), vec3(  0, 1, -1 )
);

// Omnidirectional shadow mapping calculation
float ShadowMappingCalcualtion( vec3 iViewToFrag )
{
  // Get vector between fragment position and light position
  vec3 frag_to_light = oFragPos - uLightPos[ uLightSourceIt ];

  // Get depht of the current fragment
  float frag_depth = length( frag_to_light );
  
  // PCF sample count, corresponding the array of offset direction size
  int samples_count = 20;  

  // Get distance from ViewPos to FragPos
  float frag_view_distance = length( iViewToFrag );

  // Set radius of the disk use to scale PCF offset directions
  float sample_disk_radius = ( 1.0 + ( ( frag_view_distance / uShadowFar ) * 30.0 ) ) / 500.0;

  // Init shadow value accumulator
  float shadow = 0.0;

  // Perform PCF with specific offset
  for( int sample_it = 0; sample_it < samples_count; sample_it ++ ) 
  { 
    // Get closest depth with corresponding direction offset from the preset offset array 
    float closest_depth = texture( uDepthCubeMap, frag_to_light + PCF_offset_directions[ sample_it ] * sample_disk_radius ).r;

    // Undo mapping [ 0 ; 1 ]
    closest_depth *= uShadowFar;

    // Shadow depth test   
    if( ( frag_depth - uShadowBias ) > closest_depth )
    {
      shadow += 1.0;
    }
  }
  shadow /= float( samples_count );

  return ( 1.0 - ( shadow * uShadowDarkness ) );
  //return texture( uDepthCubeMap, frag_to_light ).r;
}

// Cook torrance D function
float DistributionGGX( vec3  iNormal,
                       vec3  iHalfway,
                       float iRoughness )
{
  float a  = iRoughness * iRoughness;
  float a2 = a * a;
  float NdotH  = clamp( dot( iNormal, iHalfway ), ZERO, 1.0 ); 
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
  float NdotV = clamp( dot( iNormal, iViewDir ), ZERO, 1.0 );
  float NdotL = clamp( dot( iNormal, iLightDir ), ZERO, 1.0 );

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
                            Material iMaterial,
                            float    iShadowFactor )
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
    vec3  F   = FresnelSchlick( clamp( dot( halfway, iViewDir ), ZERO, 1.0 ), iF0 );
    float G   = GeometrySmith( iNormal, iViewDir, light_dir, iMaterial._roughness );      
    
    vec3  nominator      = NDF * F * G; 
    float denominator    = ( IV_N_dot_V * clamp( dot( iNormal, light_dir ), ZERO, 1.0 ) ) + 0.001; // 0.001 to prevent divide by zero.
    vec3  light_specular = nominator / denominator;
        

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
    float normal_dot_light_dir = clamp( dot( iNormal, light_dir ), ZERO, 1.0 ); 
    

    // Final point light influence
    // ---------------------------

    // shadow influence
    float shadow_factor = 1.0;
    if( i == uLightSourceIt )
    {
      shadow_factor = iShadowFactor;
    } 
    else
    {
      shadow_factor = min( iShadowFactor + 0.25, 1.0 );
    }

    Lo += shadow_factor * ( ( kD * ( albedo_by_PI ) ) + light_specular ) * light_radiance * normal_dot_light_dir;  // already multiplied the specular by the Fresnel ( kS )
  }   

  return Lo * iMaterial._ao;
} 

// PBR IBL calculation
vec3 IBLAmbientReflectance( float    iNormalDotViewDir,
                            Material iMaterial,
                            vec3     iF0,
                            vec3     iNormal,
                            vec3     iViewDir,
                            float    iOpacity )
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

  // Get reflection vector using parallax corrected cubemap
  vec3 reflect_dir;
  if( uParallaxCubemap )
  {
    vec3 DirectionWS = oFragPos - uViewPos;
    vec3 ReflDirectionWS = reflect( DirectionWS, iNormal);

    vec3 bmax;
    vec3 bmin;
    vec3 pos; 

    bmax = vec3( 8.0, 8.0 / 3.0, 8.0 );
    bmin = vec3( 0.0, 0.0, 0.0 );
    pos  = vec3( -8.0 * 0.5, 0.0, -8.0 * 0.5 ) + vec3( uCubemapPos.x, 0.0, uCubemapPos.z );

    bmax += pos;
    bmin += pos;

    vec3 FirstPlaneIntersect = ( bmax - oFragPos ) / ReflDirectionWS;
    vec3 SecondPlaneIntersect = ( bmin - oFragPos ) / ReflDirectionWS;  

    vec3 FurthestPlane = max( FirstPlaneIntersect, SecondPlaneIntersect );
    float Distance = min( min( FurthestPlane.x, FurthestPlane.y ), FurthestPlane.z );
    vec3 IntersectPositionWS = oFragPos + ReflDirectionWS * Distance;
    reflect_dir = IntersectPositionWS - uCubemapPos;
  }
  else
  {
    reflect_dir = reflect( -iViewDir, iNormal ); 
  }

  // Sample specular pre filtered color with a mip level corresponding to the given roughness
  vec3 prefiltered_color = textureLod( uPreFilterCubeMap, reflect_dir,  iMaterial._roughness * uMaxMipLevel ).rgb; 

  // Sample the BRDF look up texture with the given angle and roughness to get the corresponding scale and bias to F0
  vec2 brdf = texture( uPreBrdfLUT, vec2( iNormalDotViewDir, iMaterial._roughness) ).rg;

  // Compute IBL specular color
  vec3 specular = prefiltered_color * ( ( kS * brdf.x ) + brdf.y );


  // Compute final ambient IBL lighting
  // ----------------------------------
  vec3 ambient = ( diffuse + specular ); 

  // No ambient occlusion for transparent object
  if( iOpacity == 1.0 )
  {
    ambient *= iMaterial._ao;
  }

  return ambient;
}

// Complete PBR lighting calculation
vec3 PBRLightingCalculation( vec3 iNormal,
                             vec3 iViewDir,  
                             vec2 iUV,
                             float iOpacity,
                             float iShadowFactor )
{
  // Get material inputs data
  Material material;
  
  material._albedo    = pow( texture( uTextureAlbedo1, iUV ).rgb, vec3( 2.2 ) );
  //material._albedo    = pow( vec3( 1.0 ), vec3( 2.2 ) );
  material._metalness = texture( uTextureMetalness1, iUV ).r;
  material._roughness = texture( uTextureRoughness1, iUV ).r;
  material._ao        = texture( uTextureAO1, iUV ).r;

  if( uID == 7.0 )
  {
    material._albedo    = pow( texture( uTextureAlbedo1, iUV ).rgb, vec3( 2.2 ) );
    //material._albedo    = pow( vec3( 1.0 ), vec3( 2.2 ) );
    material._metalness = texture( uTextureMetalness1, iUV ).r;
    material._roughness = 1.0 - texture( uTextureMetalness1, iUV ).a;
    material._ao        = 1.0;    
  }

  // Get surface base reflectivity value
  vec3 F0 = vec3( 0.04 ); 
  F0 = mix( F0, material._albedo, material._metalness );  

  // Pre calcul N dot V
  float N_dot_V = clamp( dot( iNormal, iViewDir ), ZERO, 1.0 );

  
  // BRDF calculation part
  // ---------------------
  
  // Compute reflectance equation calculation to each scene point light
  vec3 point_lights_reflectance = PointLightReflectance( iUV,
                                                         iViewDir,
                                                         iNormal,
                                                         F0,
                                                         N_dot_V,
                                                         material,
                                                         iShadowFactor );


  // IBL calculation part
  // --------------------
   
  // Compute indirect irradiance
  vec3 IBL_ambient_reflectance = IBLAmbientReflectance( N_dot_V,
                                                        material,
                                                        F0,
                                                        iNormal,
                                                        iViewDir,
                                                        iOpacity );

  if( !uIBL )
  {
    IBL_ambient_reflectance = vec3( 0.0 );
  }


  // Return fragment final PBR lighting 
  // ----------------------------------
  return clamp( ( IBL_ambient_reflectance * min( iShadowFactor + 0.8, 1.0 ) ), ZERO, 1.0 ) + point_lights_reflectance;
}


// Main function
// -------------
void main()
{ 
  // Get fragment opacity
  float opacity;
  if( uOpacityMap )
  {
    opacity = texture( uTextureOpacity1, oUV ).r;
  }
  else
  {
    opacity = uAlpha;
  }

  // Discard for depth peeling
  if( uOpacityDiscard == 1.0 && opacity < 1.0f )
  {
    discard;
  }
  if( uOpacityDiscard == 2.0 && opacity == 1.0f )
  {
    discard;
  }

  // Get fragment normal
  vec3 normal;
  if( uNormalMap )
  {
    normal = NormalMappingCalculation( oUV );
  }
  else
  {
    normal = oNormal;
  }

  // Inverse normal for double sided mesh
  if( !gl_FrontFacing )
  {
    normal *= -1.0;
  }

  // Get view direction
  vec3 view_to_frag = uViewPos - oFragPos;
  vec3 view_dir = normalize( view_to_frag );

  // Omnidirectional shadow mapping calculation
  float shadow_factor = 1.0;
  if( uReceivShadow )
  {
    shadow_factor = ShadowMappingCalcualtion( view_to_frag );
  }

  // PBR lighting calculation 
  vec3 PBR_lighting_result = PBRLightingCalculation( normal,
                                                     view_dir,  
                                                     oUV,
                                                     opacity,
                                                     shadow_factor );

   // Get emissive value
  vec3 emissive = vec3( 0.0 );
  if( uEmissive )
  {
    emissive = texture( uTextureEmissive1, oUV ).rgb * uEmissiveFactor;
  }

  // Get final fragment color
  vec3 final_color = PBR_lighting_result + emissive;

  if( !uIBL )
  {
    final_color *= 2.0;
  }

  // Main out color
  FragColor = vec4( final_color, opacity );

  // Second out color => draw only brightest fragments
  vec3 bright_color = vec3( 0.0 );
  if( uBloom )
  {
    float brightness = dot( final_color, vec3( 0.2126, 0.7152, 0.0722 ) );
    if( brightness > uBloomBrightness )
    {
      bright_color = final_color;
    }
  }
  FragColorBrightness = vec4( bright_color, opacity );
}
