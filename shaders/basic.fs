#version 330 
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;


#define NB_LIGHTS 10
#define G_SCATTERING 0.2
#define PI 3.14159265358979323846264338
#define NB_STEPS 10

struct LightRes {    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular; 
};

//in vec3 vsoColor;
in vec2 TexCoord;
in vec3 vsoNormal;
in vec3 FragPos;
in vec4 position_for_tex;
in vec3 Tangent;
in vec4 FragPosLightSpace;
smooth in vec4 EyeSpacePos;


//out vec4 fragColor;


uniform vec3 LightPos[NB_LIGHTS];
uniform vec3 LightColor[NB_LIGHTS];
uniform vec3 LightSpecularColor[NB_LIGHTS];
uniform float constant[NB_LIGHTS];
uniform float linear[NB_LIGHTS];
uniform float quadratic[NB_LIGHTS];

uniform vec3 viewPos;

uniform float ShiniSTR;
uniform float ambientSTR;
uniform float diffuseSTR;
uniform float specularSTR;
uniform float shadow_darkness;

uniform float alpha;
uniform float var;
uniform float depth_test;
uniform float face_cube;

uniform float factor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;
uniform sampler2D texture_normal1;
uniform sampler2D texture_metalness;
uniform sampler2D depth_map_particle;
uniform sampler2D tex_render_particle;
uniform samplerCube reflection_cubeMap;
uniform sampler2D shadow_map1;
uniform samplerCube shadow_cube_map;



LightRes LightCalculation(int num_light, vec3 norm, vec3 color, vec3 light_color, vec3 light_specular_color){

  LightRes res;

  // ambient
  vec3 ambient = ambientSTR * color * light_color; 

  //diffuse
  vec3 diffuse = vec3(0.0,0.0,0.0); 
  vec3 lightDir = normalize(LightPos[num_light] - FragPos);          
  if(diffuseSTR > 0.0){
    float diff = max(dot(norm, lightDir),0.0);
    diffuse = diff * (diffuseSTR) * color * light_color;
  }
  //specular
  vec3 specular = vec3(0.0,0.0,0.0);
  if(specularSTR > 0.0){
    vec3 viewDir = normalize(viewPos - FragPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(norm, halfwayDir), 0.0), ShiniSTR);     
    
    specular = spec * specularSTR * light_specular_color; 

    // add specular mapping
    /*if(var == 1.0)
      specular *= texture(texture_specular1, TexCoord).r;
    if(var == 5.0)
      specular *= texture(texture_metalness, TexCoord).r;*/    

    /*vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), ShiniSTR);
    vec3 specular = (spec * specularSTR) * LightSpecularColor[num_light];*/ 

  }

  // Attenuation
  float distance = length(LightPos[num_light] - FragPos);
  float attenuation = 1.0f / (constant[num_light] + linear[num_light] * distance + quadratic[num_light] * (distance * distance)); 


  res.ambient = ambient;
  res.diffuse = diffuse;
  res.specular = specular;

  res.ambient *= attenuation;  
  res.diffuse *= attenuation;
  res.specular *= attenuation;  


  return res;
}




vec3 normal_mapping_calculation()                                                                     
{                                                               
    float fact = 1.0;
    
    vec3 Normal = normalize(vsoNormal);                                                       
    vec3 temp_tangent;

    if(var == 1.0){
      temp_tangent = normalize(vec3(1.0,0.0,0.0));                                                     
    }else{
      temp_tangent = normalize(Tangent);                                                     
    }
    
    temp_tangent = normalize(temp_tangent - dot(temp_tangent, Normal) * Normal);                           
    vec3 Bitemp_tangent = cross(temp_tangent, Normal);                                                
    vec3 BumpMapNormal = texture(texture_normal1, TexCoord*fact).xyz;                                
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);                              
    vec3 NewNormal;                                                                         
    mat3 TBN = mat3(temp_tangent, Bitemp_tangent, Normal);                                            
    NewNormal = TBN * BumpMapNormal;                                                        
    NewNormal = normalize(NewNormal);                                                       
    return NewNormal;                                                                       
}




void main(void) {
  // re init
  //gl_FragDepth = gl_FragCoord.z;

  if(depth_test == 0.0){

    vec3 color;
    float final_alpha;

    color = texture(texture_diffuse1, TexCoord).rgb;  
    

    if(var == 0.0){
      //color = texture(texture_normal1, TexCoord).rgb;
      //color = vec3(1.0);
    }


    final_alpha = alpha;


    // LIGHT CALCULATION
    vec3 norm = normalize(vsoNormal);
    // normal mapping
    if(var == 1.0 || var == 0.0){
       norm = normal_mapping_calculation();
    }

    LightRes LightRes1 = LightCalculation(0,norm,color,LightColor[0],LightSpecularColor[0] /*vec3(0.0,0.0,1.0)*/);



    // FINAL LIGHT
    vec3 result = (LightRes1.ambient + LightRes1.diffuse + LightRes1.specular);
   


    // ADD AO mapping
    /*if(){
      vec3 temp_AO = texture(texture_specular1, TexCoord).rgb;
      float temp = (temp_AO.r + temp_AO.g + temp_AO.b) / 3.0;
      result *= temp;
      if(var == 2.0)
        result *= temp;
    }*/


    FragColor = vec4(result, final_alpha);

    // second out => draw only brighest fragments
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.7)
        BrightColor = vec4(result, 1.0);
    


  }/*else{
    if(face_cube != -1.0){
     float far = 1000.0;
     if(var == 5.0)
      far = 1000.0 * 1.15;

     float lightDistance = length(FragPos - viewPos);
     lightDistance /= far;
     gl_FragDepth = lightDistance; //near * far / ((gl_FragCoord.z * (far - near)) - far);          
    }
  }*/
  
}
