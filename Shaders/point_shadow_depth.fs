#version 410 core


//******************************************************************************
//**********  Fragment shader inputs/ouputs  ***********************************
//******************************************************************************


// Fragment inputs from geometry shader 
// ------------------------------------
in vec4 oFragPos;


// Fragment input uniforms
// -----------------------
uniform vec3  uLightPosition;
uniform float uShadowFar;


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************

void main()
{ 
  // Light to fragment distance calculation 
  float light_distance = length( oFragPos.xyz - uLightPosition );

  // Normalize [ 0, 1 ] by dividing by uShadowFar
  light_distance = light_distance / uShadowFar;

  // write this as modified depth
  gl_FragDepth = light_distance;
}
