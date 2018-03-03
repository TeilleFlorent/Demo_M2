#version 330 core

#define PI 3.14159265358979323846264338
#define ZERO 0.00390625


//******************************************************************************
//**********  Fragment shader inputs/ouputs  ***********************************
//******************************************************************************


// Fragment color output(s)
// ------------------------
layout ( location = 0 ) out vec4 FragColor;


// Fragment input uniforms
// -----------------------
uniform samplerCube uEnvironmentMap;
uniform float       uRoughness;
uniform float       uCubeMapRes;
uniform uint        uSampleCount;


// Fragment inputs from vertex shader 
// ----------------------------------
in vec3 oWorldPos;


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************


// Low discrepancy sequence generation functions
// --------------------------------------------- 
float RadicalInverseVDC( uint iBits ) 
{
  iBits = ( iBits << 16u ) | ( iBits >> 16u );
  iBits = ( ( iBits & 0x55555555u ) << 1u ) | ( ( iBits & 0xAAAAAAAAu ) >> 1u );
  iBits = ( ( iBits & 0x33333333u ) << 2u ) | ( ( iBits & 0xCCCCCCCCu ) >> 2u );
  iBits = ( ( iBits & 0x0F0F0F0Fu ) << 4u ) | ( ( iBits & 0xF0F0F0F0u ) >> 4u );
  iBits = ( ( iBits & 0x00FF00FFu ) << 8u ) | ( ( iBits & 0xFF00FF00u ) >> 8u );
  
  return float( iBits ) * 2.3283064365386963e-10; // / 0x100000000
}
vec2 Hammersley( uint iNum, 
                 uint iCount )
{
  return vec2( float( iNum ) / float( iCount ), RadicalInverseVDC( iNum ) );
}


// Importance sampling function
// ----------------------------
vec3 ImportanceSampleGGX( vec2 iSeqValue,
                          vec3 iNormal, 
                          float iRoughness)
{
  float a = iRoughness * iRoughness;
  
  // Get halfway vector spherical coord from roughness specular lobe, and apply sequence value on it => become then a biased halfway vector
  float phi       = 2.0 * PI * iSeqValue.x;
  float cos_theta = sqrt( ( 1.0 - iSeqValue.y ) / ( 1.0 + ( a * a - 1.0 ) * iSeqValue.y) );
  float sin_theta = sqrt( 1.0 - cos_theta * cos_theta );
  
  // Get biased halfway vector into cartesian coordinates from spherical coordinates 
  vec3 temp_biased_halfway;
  temp_biased_halfway.x = cos( phi ) * sin_theta;
  temp_biased_halfway.y = sin( phi ) * sin_theta;
  temp_biased_halfway.z = cos_theta;
  
  // Get tangent and bitangent  
  vec3 up        = abs( iNormal.z ) < 0.999 ? vec3( 0.0, 0.0, 1.0 ) : vec3( 1.0, 0.0, 0.0 );
  vec3 tangent   = normalize( cross( up, iNormal ) );
  vec3 bitangent = cross( iNormal, tangent );
    
  // Get biased halfway vector to world space from tangent space
  vec3 biased_halfway = tangent * temp_biased_halfway.x + bitangent * temp_biased_halfway.y + iNormal * temp_biased_halfway.z;

  return normalize( biased_halfway );
}


// Normal distribution function, allow to generate pdf and mip level used to sample environment map
// ------------------------------------------------------------------------------------------------
float DistributionGGX( vec3  iNormal,
                       vec3  iHalfway,
                       float iRoughness )
{
  float a  = iRoughness * iRoughness;
  float a2 = a * a;
  float NdotH  = max( dot( iNormal, iHalfway ), ZERO );
  float NdotH2 = NdotH * NdotH;

  float nom   = a2;
  float denom = ( NdotH2 * ( a2 - 1.0 ) + 1.0 );
  denom = PI * denom * denom;
  
  return nom / denom;
}


// Pre filtering calculation function
// ----------------------------------
vec3 PreFilterConvolution()
{
  // Get cube fragment normal
  vec3 normal = normalize( oWorldPos );

  // Make the simplyfying assumption that view_direction vector equals reflect vector equals the normal vector 
  vec3 view_dir = normal;
  vec3 reflect  = normal;

  vec3 prefiltered_color = vec3( 0.0 );
  float total_weight     = 0.0;

  // Perform pre filtered color calculation using sample vectors biased towards the preferred alignment direction ( importance sampling )
  for( uint sample_it = 0u; sample_it < uSampleCount; sample_it++ )
  {
    // Get value from the low discrepancy sequence
    vec2 seq_value = Hammersley( sample_it,
                                 uSampleCount );

    // Get biased halfway vector using importance sampling with sequence value and roughness
    vec3 biased_halfway = ImportanceSampleGGX( seq_value, 
                                               normal, 
                                               uRoughness );

    // Get biased light direction from biased halfway and view direction
    // It will be used to sample the input cubemap
    vec3 light_dir = normalize( 2.0 * dot( view_dir, biased_halfway ) * biased_halfway - view_dir );
    
    // Get corresponding sample NdotL angle
    float N_dot_L = max( dot( normal, light_dir ), ZERO );

    if( N_dot_L > 0.0 )
    {
      // Get mip level used to sample environment map using probability density function
      float NDF = DistributionGGX( normal,
                                   biased_halfway,
                                   uRoughness );
      float N_dot_H   = max( dot( normal, biased_halfway ), ZERO );
      float H_dot_V   = max( dot( biased_halfway, view_dir ), ZERO );
      float pdf       = NDF * N_dot_H / ( 4.0 * H_dot_V ) + 0.0001;
      float sa_texel   = 4.0 * PI / ( 6.0 * uCubeMapRes * uCubeMapRes );
      float sa_sample  = 1.0 / ( float( uSampleCount ) * pdf + 0.0001 );
      float mip_level = uRoughness == 0.0 ? 0.0 : 0.5 * log2( sa_sample / sa_texel );

      vec3 sample_color = textureLod( uEnvironmentMap, light_dir, mip_level ).rgb * N_dot_L;
      if( length( sample_color ) > 0 )
      {
        // Sample environment map 
        prefiltered_color += sample_color;

        // Add corresponding sample weight ( sample with small N_dot_L == small influence )
        total_weight += N_dot_L;
      }
    }
  }

  return prefiltered_color / total_weight;
}


// Main function
// -------------
void main()
{		
  FragColor = vec4( PreFilterConvolution(), 1.0 );
}
