#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;


out vec2 TexCoord;
out vec3 Position;
/* out vec4 vsoColor; */

uniform float x_num;
uniform float y_num;
uniform float atlas_size;

void main(void) {

	gl_Position = vec4(vec3((position.x),position.y,position.z), 1.0); 
 
	TexCoord = vec2(texCoord.x, texCoord.y);

  //float chunk_size =  (1.0/atlas_size);
  //TexCoord.x = (chunk_size * x_num) + (TexCoord.x * chunk_size);
  //TexCoord.y = (chunk_size * y_num) + (TexCoord.y * chunk_size);

	Position = position;
  

}
