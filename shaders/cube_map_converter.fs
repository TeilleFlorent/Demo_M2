#version 330


//******************************************************************************
//**********  Fragment shader inputs/ouputs  ***********************************
//******************************************************************************


// Fragment color output(s)
// ------------------------
layout ( location = 0 ) out vec4 FragColor;


// Fragment input uniforms
// -----------------------
uniform sampler2D uEquirectangularMap;


// Fragment inputs from vertex shader 
// ----------------------------------
in vec3 oWorldPos;


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************

const vec2 inverse_Atan = vec2( 0.1591, 0.3183 );

vec2 SampleSphericalMap( vec3 iWorldPos )
{
  vec2 uv = vec2( atan( iWorldPos.z, iWorldPos.x ), asin( iWorldPos.y ) );
  uv *= inverse_Atan;
  uv += 0.5;
  return uv;
}

void main()
{		
  vec2 uv = SampleSphericalMap( normalize( oWorldPos ) );
  vec3 color_result = texture( uEquirectangularMap, uv).rgb;
  FragColor = vec4( color_result, 1.0 );
}
