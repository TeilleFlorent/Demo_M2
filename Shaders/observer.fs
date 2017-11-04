#version 330


//******************************************************************************
//**********  Fragment shader inputs/ouputs  ***********************************
//******************************************************************************


// Fragment color output(s)
// ------------------------
layout ( location = 0 ) out vec4 FragColor;


// Fragment input uniforms
// -----------------------
uniform float uCameraNear;
uniform float uCameraFar;
uniform sampler2D uTexture1;


// Fragment inputs from vertex shader 
// ----------------------------------
in vec2 oUV;


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************

// probably doesn't work
float LinearDepth( float iDepthSample )
{
  float n = uCameraNear; // camera z near
  float f = uCameraFar;  // camera z far
  float z = iDepthSample;
  return ( 2.0 * n ) / ( f + n - z * ( f - n ) );
}

vec4 TextureMultisample( sampler2DMS iSampler,
                         ivec2 iPosition )
{
  vec4 color = vec4( 0.0 );
  
  for( int i = 0; i < 4; i++ )
  {
    color += texelFetch( iSampler, iPosition, i );
  }
  color /= float( 4 );
  
  return color;
}

void main()
{
  vec3 color_result = texture( uTexture1, oUV ).rgb;

  /*ivec2 TC = ivec2(floor(textureSize(texture1) * TexCoord)); 
  result = TextureMultisample(texture1, TC).rgb;*/


  /*float temp = texture(texture1, TexCoord).r;
  result = vec3((temp));*/
  //result = vec3(linearDepth(temp));

  FragColor = vec4( color_result, 1.0 );
  //FragColor = vec4( vec3( color_result.b ), 1.0 );
}
