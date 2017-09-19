#version 330


//******************************************************************************
//**********  Fragment shader inputs/ouputs  ***********************************
//******************************************************************************


// Fragment color output(s)
// ------------------------
layout ( location = 0 ) out vec4 FragColor;


// Fragment input uniforms
// -----------------------
uniform sampler2D uBaseColorTexture;
uniform sampler2D uBloomBrightnessTexture;
uniform bool uBloom;
uniform float uExposure;


// Fragment inputs from vertex shader 
// ----------------------------------
in vec2 oUV;


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************

void main()
{             
  const float gamma = 2.2;

  vec3 hdr_base_color = texture( uBaseColorTexture, oUV ).rgb;      
  vec3 bloom_color = texture( uBloomBrightnessTexture, oUV ).rgb;

  if( uBloom )
  {
    // Bloom additive blending
    hdr_base_color += bloom_color; 
  }

  // Tone mapping post process
  vec3 color_result = vec3( 1.0 ) - exp( -hdr_base_color * uExposure );

  // Gamma correction post process       
  color_result = pow( color_result, vec3( 1.0 / gamma ) );

  // Main out color
  FragColor = vec4( color_result, 1.0f );
}