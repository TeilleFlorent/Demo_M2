#version 330

#define MAX_NB_LIGHTS 20


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
uniform mat4 uLightSpaceMatrix;
uniform mat4 uCubeMapViewMatrices[ 6 ];
uniform float uShiniSTR;
uniform float uID;
uniform float uCubeMapFaceNum;

uniform vec3 uViewPos;
uniform vec3 uLightPos[ MAX_NB_LIGHTS ];


// Vertex outputs to fragment shader	
// ---------------------------------
out vec3 oNormal;
out vec2 oUV;
out vec3 oFragPos;
out vec3 oViewSpaceFragPos;
out vec4 oClipSpacePosition;
out vec3 oTangentLightPos[ MAX_NB_LIGHTS ];
out vec3 oTangentViewPos;
out vec3 oTangentFragPos;
flat out vec3 oTBN[ 3 ];
 

//******************************************************************************
//**********  Vertex shader functions  *****************************************
//******************************************************************************

void main()
{

	// Vertex position calculation
	// ---------------------------
	vec4 clip_space_position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4( _position, 1.0 );
	gl_Position = clip_space_position;


	// Vertex TBN matrix calculation
	// -----------------------------
	vec3 N = normalize( mat3( uModelMatrix ) * _normal );
	vec3 T = normalize( mat3( uModelMatrix ) * _tangent );
	vec3 B = normalize( mat3( uModelMatrix ) * _bitangent );
	mat3 TBN;
	TBN = transpose( mat3( T, B, N ) );


	// Vertex outputs calculation
	// --------------------------
	oClipSpacePosition = clip_space_position; 

	oFragPos = vec3( uModelMatrix * vec4( _position, 1.0f ) );
	
	oViewSpaceFragPos = vec3( uViewMatrix * uModelMatrix * vec4( _position, 1.0f ) );
	
	oUV = _uv;

	oNormal = N;  

	oTangentViewPos  = TBN * uViewPos;
	oTangentFragPos  = TBN * oFragPos;

	for( int i = 0; i < uLightCount; i++ )
	{
		//oTangentLightPos[ i ] = TBN * uLightPos[ i ];
	}

	oTangentLightPos[ 0 ] = TBN * uLightPos[ 0 ];

	oTBN[ 0 ] = TBN[ 0 ];
	oTBN[ 1 ] = TBN[ 1 ];
	oTBN[ 2 ] = TBN[ 2 ];
}

