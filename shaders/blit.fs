#version 330


//******************************************************************************
//**********  Fragment shader inputs/ouputs  ***********************************
//******************************************************************************


// Fragment color output(s)
// ------------------------
layout ( location = 0 ) out vec4 FragColor;


// Fragment input uniforms
// -----------------------
uniform sampler2DMS uTexture1;
uniform int uSampleCount;


// Fragment inputs from vertex shader 
// ----------------------------------
in vec2 oUV;


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************

vec4 TextureMultisample( sampler2DMS iSampler,
                         ivec2 iPosition )
{
  vec4 color = vec4( 0.0 );

  for( int i = 0; i < uSampleCount; i++ )
  {
    color += texelFetch( iSampler, iPosition, i );
  }
  color /= float( uSampleCount );

  return color;
}

void main()
{
  ivec2 uv = ivec2( floor( textureSize( uTexture1 ) * oUV ) ); 
  vec3 color_result = TextureMultisample( uTexture1, uv ).rgb;

  FragColor = vec4( color_result, 1.0 );
}
