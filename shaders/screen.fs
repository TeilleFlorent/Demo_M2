#version 330

in vec2 TexCoord;
in vec3 Positon;

uniform float output_factor;

out vec4 fragColor;
uniform sampler2D depth_map_feu;



void main(void) {

  vec3 result;
  float final_alpha = 1.0;

  //result = vec3(0.0,0.0,1.0);
  //float temp_depth = texture(depth_map_feu, TexCoord).r;
  //temp_depth = 0.1 * 1000.0 / ((temp_depth * (1000.0 - 0.1)) - 1000.0);
            
  //result = vec3(temp_depth);
  
  result = texture(depth_map_feu, TexCoord).rgb;
  final_alpha = texture(depth_map_feu, TexCoord).a;


  //result = texture(tex_particle, TexCoord).rgb;
  //final_alpha = texture(tex_particle, TexCoord).a;

  //result = vec3(0.0,0.0,0.0);
  //final_alpha = 1.0;
  
  //result *= output_factor;
  
  fragColor = vec4(result, final_alpha);
  
}
