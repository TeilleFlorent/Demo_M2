#version 330


//******************************************************************************
//**********  Vertex shader inputs/ouputs  *************************************
//******************************************************************************


// Vertex input attributes
// -----------------------
layout (location = 0) in vec3 _posisition;


// Vertex input uniforms
// ---------------------
uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;


// Vertex outputs to fragment shader  
// ---------------------------------
out vec3 oWorldPos;


//******************************************************************************
//**********  Vertex shader functions  *****************************************
//******************************************************************************

void main()
{
  oWorldPos   = _posisition;  
  gl_Position = uProjectionMatrix * uViewMatrix * vec4( oWorldPos, 1.0 );
}