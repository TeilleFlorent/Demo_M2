#version 330

in vec2 TexCoord;
in vec3 Positon;

uniform float output_factor;
uniform float camera_near;
uniform float camera_far;


out vec4 fragColor;
uniform sampler2D depth_map_feu;



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



void main(void) {

  vec3 result;
  float final_alpha = 1.0;

  
  result = texture(depth_map_feu, TexCoord).rgb;
  final_alpha = texture(depth_map_feu, TexCoord).a;


  /*float temp = texture(depth_map_feu, TexCoord).r;
  result = vec3((temp));*/
  //result = vec3(linearDepth(temp));
  

  fragColor = vec4(result, final_alpha);
  
}
