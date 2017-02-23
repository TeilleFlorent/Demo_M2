#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;


out vec2 TexCoord;
out vec3 Position;


void main(void) {

	gl_Position = vec4(vec3((position.x),position.y,position.z), 1.0); 
 
	TexCoord = vec2(texCoord.x, texCoord.y);

	Position = position;
  

}
