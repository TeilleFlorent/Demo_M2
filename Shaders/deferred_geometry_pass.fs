#version 330 


//******************************************************************************
//**********  Fragment shader inputs/ouputs  ***********************************
//******************************************************************************


// Fragment color output(s)
// ------------------------
layout( location = 0 ) out vec3 FragColorPosition;
layout( location = 1 ) out vec3 FragColorNormal;
layout( location = 2 ) out vec3 FragColorAlbedo;
layout( location = 3 ) out vec3 FragColorRougnessMetalnessAO;


// Fragment input uniforms
// -----------------------
uniform sampler2D uTextureDiffuse1; 
uniform sampler2D uTextureSpecular1;
uniform sampler2D uTextureNormal1; 
uniform sampler2D uTextureHeight1; 
uniform sampler2D uTextureAO1; 
uniform sampler2D uTextureRoughness1; 
uniform sampler2D uTextureMetalness1;


// Fragment inputs from vertex shader 
// ----------------------------------
in vec2 oUV;
in vec3 oNormal;
in vec3 oFragPos;
flat in vec3 oTBN[ 3 ];


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************


// Normal mapping function
// -----------------------
vec3 NormalMappingCalculation( vec2 iUV )                                                                     
{      
  vec3 res_normal;
      
  // Get normal vector
  res_normal = texture( uTextureNormal1, iUV ).rgb;
  res_normal = normalize( res_normal * 2.0 - 1.0 );   

  // Get TBN matrix
  mat3 TBN;
  TBN[ 0 ] = oTBN[ 0 ];
  TBN[ 1 ] = oTBN[ 1 ];
  TBN[ 2 ] = oTBN[ 2 ];

  return normalize( res_normal * TBN );
}

void main()
{ 

  // G Buffer drawing
  // ----------------
  
  // Draw G buffer position
  FragColorPosition = oFragPos;

  // Draw G buffer normal
  FragColorNormal = NormalMappingCalculation( oUV );

  // Draw G buffer albedo
  FragColorAlbedo = pow( texture( uTextureDiffuse1, oUV ).rgb, vec3( 2.2 ) );
  //FragColorAlbedo = pow( vec3( 1.0, 1.0, 1.0 ), vec3( 2.2 ) );

  // Draw G buffer roughness
  FragColorRougnessMetalnessAO.r = texture( uTextureRoughness1, oUV ).r;

  // Draw G buffer metalness
  FragColorRougnessMetalnessAO.g = texture( uTextureMetalness1, oUV ).r;   

  // Draw G buffer AO
  FragColorRougnessMetalnessAO.b = texture( uTextureAO1, oUV ).r;
}
