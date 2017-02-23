#version 330

in vec2 TexCoord;
in vec3 Positon;


out vec4 fragColor;
uniform sampler2DMS texture1;
uniform int nb_sample;


vec4 textureMultisample(sampler2DMS sampler, ivec2 ipos)
{
  vec4 color = vec4(0.0);
  
  for (int i = 0; i < nb_sample; i++)
  {
    color += texelFetch(sampler, ipos, i);
  }
  
  color /= float(nb_sample);
  
  return color;
}



void main(void) {

  vec3 result;
  float final_alpha = 1.0;


  /*result = texture(texture1, TexCoord).rgb;
  final_alpha = texture(texture1, TexCoord).a; */ 

  ivec2 TC = ivec2(floor(textureSize(texture1) * TexCoord)); 
  result = textureMultisample(texture1, TC).rgb;
  //final_alpha = textureMultisample(texture1, TC).a;


  /*float temp = texture(texture1, TexCoord).r;
  result = vec3((temp));*/
  //result = vec3(linearDepth(temp));
  

  fragColor = vec4(result, final_alpha);
  
}
