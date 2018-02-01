#version 410 core


//******************************************************************************
//**********  Vertex shader inputs/ouputs  *************************************
//******************************************************************************


// Vertex input attributes
// -----------------------
layout ( location = 0 ) in vec3 _position;
layout ( location = 1 ) in vec3 _normal;
layout ( location = 2 ) in vec2 _uv;


// Vertex input uniforms
// ---------------------
uniform mat4 uModelMatrix;


// Vertex outputs to tessellation control shader	
// ---------------------------------------------
out vec3 oFragPosToCS;
out vec2 oUVToCS;
out vec3 oNormalToCS;


//******************************************************************************
//**********  Vertex shader functions  *****************************************
//******************************************************************************

void main()
{
	oFragPosToCS = ( uModelMatrix * vec4( _position, 1.0 ) ).xyz;
	oUVToCS 		 = _uv;
	oNormalToCS  = ( uModelMatrix * vec4( _normal, 0.0 ) ).xyz;
}

