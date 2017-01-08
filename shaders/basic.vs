#version 330

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 lightSpaceMatrix;
uniform mat4 cubeMap_viewMatrices[6];
uniform float ShiniSTR;
uniform float var;
uniform float face_cube;

layout (location = 0) in vec3 vsiPosition;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tangent;

 out vec3 vsoNormal;
 out vec2 TexCoord;
 out vec3 FragPos;
 out vec4 position_for_tex;
 out vec3 Tangent;
 out vec4 FragPosLightSpace;
 smooth out vec4 EyeSpacePos;

 void main(void) {

  //vsoNormal = (transpose(inverse(modelViewMatrix))  * vec4(vsiPosition, 0.0)).xyz;

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

 	FragPos=vec3(modelViewMatrix2 * vec4(vsiPosition,1.0f));

 	vsoNormal = mat3(transpose(inverse(modelViewMatrix2))) * normal;  
 
	TexCoord = texCoord; //vec2(texCoord.x, 1.0 - texCoord.y);

	if(var == 2.0){
		if(ShiniSTR == 8.0)
			TexCoord = texCoord * 0.75; 
 	}

	Tangent = vec3(modelViewMatrix2 * vec4(tangent, 0.0));

	if(var == 0.0 || var == 2.0 || var == 4.0){
		FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
	}

	EyeSpacePos =  (viewMatrix * modelViewMatrix2) * vec4(vsiPosition, 1.0);

}

