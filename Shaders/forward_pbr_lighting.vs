#version 330

#define MAX_NB_LIGHTS 25


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
uniform int uLightCount;

uniform mat4 uModelMatrix;
uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform float uID;

uniform vec3 uViewPos;
uniform vec3 uLightPos[ MAX_NB_LIGHTS ];


// Vertex outputs to fragment shader	
// ---------------------------------
out vec2 oUV;
out vec3 oFragPos;
out vec3 oTBN[ 3 ];
out vec3 oNormal;

//******************************************************************************
//**********  Vertex shader functions  *****************************************
//******************************************************************************

void main()
{

	// Vertex position calculation
	// ---------------------------
	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4( _position, 1.0 );


	// Vertex TBN matrix calculation
	// -----------------------------
	vec3 N = normalize( mat3( uModelMatrix ) * _normal );
	vec3 T = normalize( mat3( uModelMatrix ) * _tangent );
	vec3 B = normalize( mat3( uModelMatrix ) * _bitangent );
	mat3 TBN;
	TBN = transpose( mat3( T, B, N ) );


	// Vertex outputs calculation
	// --------------------------
	oFragPos = vec3( uModelMatrix * vec4( _position, 1.0f ) );
	
	oUV = _uv;

	oTBN[ 0 ] = TBN[ 0 ];
	oTBN[ 1 ] = TBN[ 1 ];
	oTBN[ 2 ] = TBN[ 2 ];

	oNormal = N;
}

