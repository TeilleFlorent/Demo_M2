#version 330


//******************************************************************************
//**********  Fragment shader inputs/ouputs  ***********************************
//******************************************************************************


// Fragment color output(s)
// ------------------------
layout ( location = 0 ) out vec4 FragColor;
layout ( location = 1 ) out vec4 FragColorBrightness;


// Fragment input uniforms
// -----------------------
uniform float uAlpha;
uniform samplerCube uSkyboxTexture;
uniform bool  uBloom;
uniform float uBloomBrightness;


// Fragment inputs from vertex shader 
// ----------------------------------
in vec3 oUV;


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************

void main()
{    
  vec3 result_color;
  result_color = texture( uSkyboxTexture, oUV ).rgb;

  // Main out color
  FragColor = vec4( result_color , uAlpha );

  // Second out color => draw only brightest fragments
  vec3 bright_color = vec3( 0.0, 0.0, 0.0 );
  if( uBloom )
  {
    float brightness = dot( result_color, vec3( 0.2126, 0.7152, 0.0722 ) );
    if( brightness > uBloomBrightness )
    {
      bright_color = result_color;
    }
  }
  FragColorBrightness = vec4( bright_color, uAlpha );
}
  