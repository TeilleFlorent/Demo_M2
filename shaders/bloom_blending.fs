#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene_color;
uniform sampler2D bloom_effect;
uniform bool bloom;
uniform float exposure;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(scene_color, TexCoords).rgb;      
    vec3 bloomColor = texture(bloom_effect, TexCoords).rgb;
    if(bloom)
        hdrColor += bloomColor; // additive blending
    
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    
    // also gamma correct while we're at it       
    result = pow(result, vec3(1.0 / gamma));
    
    FragColor = vec4(result, 1.0f);
}