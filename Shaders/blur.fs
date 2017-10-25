#version 330 core


//******************************************************************************
//**********  Fragment shader inputs/ouputs  ***********************************
//******************************************************************************


// Fragment color output(s)
// ------------------------
layout ( location = 0 ) out vec4 FragColor;


// Fragment input uniforms
// -----------------------
uniform sampler2D uTexture1;
uniform float uHorizontal;
uniform float uOffsetFactor;


// Fragment inputs from vertex shader 
// ----------------------------------
in vec2 oUV;


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************

float weight[ 5 ] = float[] ( 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 );

void main()
{
  vec2 tex_offset = 1.0 / textureSize( uTexture1, 0 ); // gets size of single texel
  tex_offset *= uOffsetFactor;

  vec3 color_result = texture(uTexture1, oUV).rgb * weight[0];

  if( uHorizontal == 1.0 )
  {
    for( int i = 1; i < 5; ++i )
    {
      color_result += texture( uTexture1, oUV + vec2( tex_offset.x * i, 0.0 ) ).rgb * weight[ i ];
      color_result += texture( uTexture1, oUV - vec2( tex_offset.x * i, 0.0 ) ).rgb * weight[ i ];
    }
  }
  else
  {
    for( int i = 1; i < 5; ++i )
    {
      color_result += texture( uTexture1, oUV + vec2( 0.0, tex_offset.y * i ) ).rgb * weight[ i ];
      color_result += texture( uTexture1, oUV - vec2( 0.0, tex_offset.y * i ) ).rgb * weight[ i ];
    }
  }

  FragColor = vec4( color_result, 1.0 );
}