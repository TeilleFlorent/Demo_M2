#version 410 core


//******************************************************************************
//**********  Vertex shader inputs/ouputs  *************************************
//******************************************************************************


// Vertex input attributes
// -----------------------
layout ( location = 0 ) in vec3 _position;
layout ( location = 1 ) in vec3 _normal;
layout ( location = 2 ) in vec2 _uv;
layout ( location = 3 ) in vec3 _tangent;
layout ( location = 4 ) in vec3 _bitangent;


// Vertex input uniforms
// ---------------------
uniform mat4 uModelMatrix;


// Vertex outputs to tessellation control shader	
// ---------------------------------------------
out vec3 oFragPosToCS;
out vec2 oUVToCS;
out vec3 oNormalToCS;
out vec3 oTangentToCS;
out vec3 oBiTangentToCS;


//******************************************************************************
//**********  Vertex shader functions  *****************************************
//******************************************************************************

void main()
{
	oFragPosToCS = ( uModelMatrix * vec4( _position, 1.0 ) ).xyz;
	oUVToCS 		 = _uv;

	// normalize out normal because the tessellation control shader relies on the normal having a unit length to generate new control points
	oNormalToCS    = normalize( mat3( uModelMatrix ) * _normal );
	oTangentToCS   = normalize( mat3( uModelMatrix ) * _tangent );
	oBiTangentToCS = normalize( mat3( uModelMatrix ) * _bitangent );
}

