#version 330

#define NB_LIGHTS 10


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
uniform vec3 LightPos[NB_LIGHTS];

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
 out vec3 TangentLightPos;
 out vec3 TangentViewPos;
 out vec3 TangentFragPos;
 out vec3 Tangent;

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
		TexCoord = texCoord * 3.0; 

	// normal calculation
	mat3 normalMatrix = mat3(transpose(inverse(modelMatrix2)));
    vsoNormal = normalize(normalMatrix * normal);
    
    Tangent = vec3(modelMatrix2 * vec4(tangent, 0.0));

	// TBN calculation
	if(var == 1.0 || var == 0.0){

		/*vec3 T;
		T = normalize(normalMatrix2 * tangent);

		if(var == 1.0){
			T = normalize(normalMatrix2 * vec3(1.0,1.0,0.0));   
		}
		if(var == 0.0){
			T = normalize(normalMatrix2 * vec3(tangent.x * 1.0f, tangent.y * 1.0f, tangent.z * 1.0f));	
		}
		vec3 N = normalize(normalMatrix2 * normal);
		T = normalize(T - dot(T, N) * N);
		vec3 B = cross(N, T);
		B = normalize(normalMatrix2 * B);
		mat3 TBN = transpose(mat3(T, B, N));*/

		vec3 N = normalize(mat3(modelMatrix2) * normal);
		//vec3 T = normalize(mat3(modelMatrix2) * (tangent * 2.0 - 1.0));
		vec3 T = normalize(mat3(modelMatrix2) * (tangent));
		vec3 B;
		mat3 TBN;
		if(var == 1.0){
			B = normalize(mat3(modelMatrix2) * bitangent);
			TBN = transpose(mat3(T, B, N));

			/*T = normalize(T - dot(T, N) * N);
			vec3 B = cross(N, T);

			//TBN = mat3(T, B, N);  
			TBN = transpose(mat3(T, B, N));*/


		}else{

			B = normalize(mat3(modelMatrix2) * bitangent);
			TBN = transpose(mat3(T, B, N));

			/*T = normalize(T - dot(T, N) * N);
			vec3 B = cross(N, T);

			//TBN = mat3(T, B, N);  
			TBN = transpose(mat3(T, B, N));*/


		}


		TangentLightPos = TBN * LightPos[0];
		TangentViewPos  = TBN * viewPos;
		TangentFragPos  = TBN * FragPos;
	}
}

