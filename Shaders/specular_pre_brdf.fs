#version 330 core

#define PI 3.14159265358979323846264338


//******************************************************************************
//**********  Fragment shader inputs/ouputs  ***********************************
//******************************************************************************


// Fragment color output(s)
// ------------------------
layout ( location = 0 ) out vec4 FragColor;


// Fragment input uniforms
// -----------------------
uniform uint uSampleCount;


// Fragment inputs from vertex shader 
// ----------------------------------
in vec2 oUV;


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


// BRDF geometry functions
// -----------------------
float GeometrySchlickGGX( float NdotV,
                          float iRoughness )
{
  float r = ( iRoughness + 1.0 );
  float k = ( r * r ) / 2.0;

  float nom   = NdotV;
  float denom = NdotV * ( 1.0 - k ) + k;

  return nom / denom;
}

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


// BRDF pre calculation function
// -----------------------------
vec2 PreBRDFConvolution( float iNdotV,
                         float iRoughness )
{
  // Get view direction vector
  vec3 view_dir;
  view_dir.x = sqrt( 1.0 - iNdotV * iNdotV );
  view_dir.y = 0.0;
  view_dir.z = iNdotV;

  float output_scale = 0.0;
  float output_bias  = 0.0; 


  return vec2( output_scale, output_bias );
}


// Main function
// -------------
void main()
{		
  FragColor = vec4( 0.0, 1.0, 0.0, 1.0 );
}
