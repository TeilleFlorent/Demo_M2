#version 330

#define MAX_NB_LIGHTS 10

uniform int nb_lights;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 projectionMatrix2;
uniform mat4 projectionMatrix3;
uniform mat4 viewMatrix;
uniform mat4 lightSpaceMatrix;
uniform mat4 cubeMap_viewMatrices[6];
uniform float ShiniSTR;
uniform float var;
uniform float face_cube;

uniform float camera_near;
uniform float camera_far;

uniform vec3 viewPos;
uniform vec3 LightPos[MAX_NB_LIGHTS];

layout (location = 0) in vec3 vsiPosition;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

 out vec3 vsoNormal;
 out vec2 TexCoord;
 out vec3 FragPos;
 out vec3 cs_FragPos;
 out vec4 position_for_tex;
 out vec3 TangentLightPos[MAX_NB_LIGHTS];
 out vec3 TangentViewPos;
 out vec3 TangentFragPos;
 flat out vec3 out_TBN[3];
 
 void main(void) {

 	mat4 modelMatrix2, viewMatrix2;

 	modelMatrix2 = modelMatrix; 

 	if(face_cube != -1.0){
 		viewMatrix2 = cubeMap_viewMatrices[int(face_cube)];
 	}else{
 		viewMatrix2 = viewMatrix;
 	}

 	vec4 out_position = projectionMatrix * viewMatrix2 * modelMatrix2 * vec4(vsiPosition, 1.0);
 
 	gl_Position = out_position;

 	position_for_tex = out_position; 

 	FragPos = vec3(modelMatrix2 * vec4(vsiPosition,1.0f));
 	cs_FragPos = vec3(viewMatrix2 * modelMatrix2 * vec4(vsiPosition,1.0f));
 	

	TexCoord = texCoord; //vec2(texCoord.x, 1.0 - texCoord.y);
	if(var == 1.0)
		TexCoord = texCoord * 5.0; 

	// normal calculation
	mat3 normalMatrix = mat3(transpose(inverse(modelMatrix2)));
    //vsoNormal = normalize(normalMatrix * normal);
    vsoNormal = mat3(modelMatrix2) * normal;  
 
	// TBN calculation
	if(/*var == 1.0 || var == 0.0*/ true){

		vec3 N = normalize(mat3(modelMatrix2) * normal);
		vec3 T = normalize(mat3(modelMatrix2) * (tangent));
		vec3 B;
		mat3 TBN;
		B = normalize(mat3(modelMatrix2) * bitangent);
		TBN = transpose(mat3(T, B, N));

		for(int i = 0; i < nb_lights; ++i){
			//TangentLightPos[i] = TBN * LightPos[i];
		}
		TangentLightPos[0] = TBN * LightPos[0];   // BUG
		TangentLightPos[1] = TBN * LightPos[1];
		TangentLightPos[2] = TBN * LightPos[2];   // BUG
		TangentLightPos[3] = TBN * LightPos[3];
		
		TangentViewPos  = TBN * viewPos;
		TangentFragPos  = TBN * FragPos;

		out_TBN[0] = TBN[0];
		out_TBN[1] = TBN[1];
		out_TBN[2] = TBN[2];

	}

}

