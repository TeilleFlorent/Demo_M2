#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;


in vec3 TexCoords;
in vec3 FragPos;

out vec4 fragColor;

uniform samplerCube skybox;
uniform float alpha;


void main(){    
	
    vec3 result,color;
    float final_alpha = 1.0;

    color = texture(skybox, TexCoords).rgb;

    result = color;
   
    
    FragColor = vec4(result , final_alpha);
    // second out => draw only brighest fragments
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.99){
        //BrightColor = vec4(result, 1.0);
        BrightColor = vec4(0.0,0.0,0.0,1.0);
    }
}
  