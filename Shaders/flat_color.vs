#version 330 


//******************************************************************************
//**********  Vertex shader inputs/ouputs  *************************************
//******************************************************************************


// Vertex input attributes
// -----------------------
layout ( location = 0 ) in vec3 _position;


// Vertex input uniforms
// ---------------------
uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;


//******************************************************************************
//**********  Vertex shader functions  *****************************************
//******************************************************************************

void main()
{
  gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4( _position, 1.0f );
} 
