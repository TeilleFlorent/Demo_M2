#version 330

in vec2 TexCoord;
in vec3 Positon;

uniform float output_factor;
uniform float camera_near;
uniform float camera_far;


out vec4 fragColor;
uniform sampler2D texture1;



float linearDepth(float depthSample)
{

/*    float z_b = depthSample;
    float z_n = 2.0 * z_b - 1.0;
    float z_e = 2.0 * camera_near * camera_far / (camera_far + camera_near - z_n * (camera_far - camera_near));
    return z_e;*/

    float n = camera_near; // camera z near
    float f = camera_far; // camera z far
    float z = depthSample;
    return (2.0 * n) / (f + n - z * (f - n));



/*    float depth = depthSample;
    return depthSample;
*/
}

vec4 textureMultisample(sampler2DMS sampler, ivec2 ipos)
{
  vec4 color = vec4(0.0);
  
  for (int i = 0; i < 4; i++)
  {
    color += texelFetch(sampler, ipos, i);
  }
  
  color /= float(4);
  
  return color;
}



void main(void) {

  vec3 result;
  float final_alpha = 1.0;



  result = texture(texture1, TexCoord).rgb;
  final_alpha = texture(texture1, TexCoord).a;  

  /*ivec2 TC = ivec2(floor(textureSize(texture1) * TexCoord)); 
  result = textureMultisample(texture1, TC).rgb;*/


  /*float temp = texture(texture1, TexCoord).r;
  result = vec3((temp));*/
  //result = vec3(linearDepth(temp));
  

  fragColor = vec4(result, final_alpha);
  
}
