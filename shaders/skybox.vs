#version 330


//******************************************************************************
//**********  Vertex shader inputs/ouputs  *************************************
//******************************************************************************


// Vertex input attributes
// -----------------------
layout ( location = 0 ) in vec3 _position;


// Vertex input uniforms
// ---------------------
uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;


// Vertex outputs to fragment shader  
// --------------------------------
out vec3 oUV;


//******************************************************************************
//**********  Vertex shader functions  *****************************************
//******************************************************************************

void main()
{
  vec4 clip_space_position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4( _position, 1.0);
  gl_Position = clip_space_position;  

  oUV = _position;
}  

