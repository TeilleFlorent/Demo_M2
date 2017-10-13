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
uniform vec3  uColor;
uniform bool  uBloom;
uniform float uBloomBrightness;


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************

void main()
{   
  // Main out color
  FragColor = vec4( uColor * 3.0, 1.0 );

  // Second out color => draw only brightest fragments
  vec3 bright_color = vec3( 0.0, 0.0, 0.0 );
  if( uBloom )
  {
    float brightness = dot( uColor, vec3( 0.2126, 0.7152, 0.0722 ) );
    if( brightness > uBloomBrightness )
    {
      bright_color = uColor;
    }
  }
  FragColorBrightness = vec4( bright_color, 1.0 );
}
