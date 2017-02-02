#version 330

#define NB_LIGHTS 10


uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 projectionMatrix2;
uniform mat4 viewMatrix;
uniform mat4 lightSpaceMatrix;
uniform mat4 cubeMap_viewMatrices[6];
uniform float ShiniSTR;
uniform float var;
uniform float face_cube;

uniform vec3 viewPos;
uniform vec3 LightPos[NB_LIGHTS];

layout (location = 0) in vec3 vsiPosition;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tangent;

 out vec3 vsoNormal;
 out vec2 TexCoord;
 out vec3 FragPos;
 out vec3 cs_FragPos;
 out vec4 position_for_tex;
 out vec3 TangentLightPos;
 out vec3 TangentViewPos;
 out vec3 TangentFragPos;

 void main(void) {

 	mat4 modelViewMatrix2, viewMatrix2;

 	modelViewMatrix2 = modelViewMatrix; 

 	if(face_cube != -1.0){
 		viewMatrix2 = cubeMap_viewMatrices[int(face_cube)];
 	}else{
 		viewMatrix2 = viewMatrix;
 	}

 	vec4 out_position = projectionMatrix * viewMatrix2 * modelViewMatrix2 * vec4(vsiPosition, 1.0);
 
 	gl_Position = out_position;

 	position_for_tex = out_position; 

 	FragPos = vec3(modelViewMatrix2 * vec4(vsiPosition,1.0f));
 	cs_FragPos = vec3(viewMatrix2 * modelViewMatrix2 * vec4(vsiPosition,1.0f));

 	
	TexCoord = texCoord; //vec2(texCoord.x, 1.0 - texCoord.y);
	if(var == 1.0)
		TexCoord = texCoord * 3.0; 

	// normal calculation
	mat3 normalMatrix = mat3(transpose(inverse(modelViewMatrix2)));
    vsoNormal = normalize(normalMatrix * normal);
	mat3 normalMatrix2 = mat3(modelViewMatrix2);
    

	// TBN calculation
	vec3 T;
	T = normalize(normalMatrix2 * tangent);
	
	if(var == 1.0){
		T = normalize(normalMatrix2 * vec3(1.0,0.0,0.0));   
	}
	if(var == 0.0){
		T = normalize(normalMatrix2 * vec3(tangent.x * 1.0f, tangent.y * 1.0f, tangent.z * 1.0f));	
	}
    vec3 N = normalize(normalMatrix2 * normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    B = normalize(normalMatrix2 * B);
    mat3 TBN = transpose(mat3(T, B, N));
    
    TangentLightPos = TBN * LightPos[0];
    TangentViewPos  = TBN * viewPos;
    TangentFragPos  = TBN * FragPos;
    
}

