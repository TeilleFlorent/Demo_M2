#version 330 
#define point2 vec2
#define point3 vec3

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragColor2;

#define NB_LIGHTS 10
#define PI 3.14159265358979323846264338

struct LightRes {    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular; 
};

//in vec3 vsoColor;
in vec2 TexCoord;
in vec3 vsoNormal;
in vec3 FragPos;
in vec3 cs_FragPos;
in vec4 position_for_tex;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

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
uniform bool SSR_pre_rendu;

uniform float camera_near;
uniform float camera_far;
uniform vec3 clip_info;
uniform mat4 projectionMatrix2;

uniform float height_scale;


uniform sampler2D texture_diffuse1; 
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1; 
uniform sampler2D texture_height1; 
uniform sampler2D texture_color_SSR;
uniform sampler2D texture_depth_SSR;
uniform float tex_x_size;
uniform float tex_y_size;



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


LightRes LightCalculation(int num_light, vec3 norm, vec3 viewDir, vec3 color, vec3 light_color, vec3 light_specular_color){

  LightRes res;

  // ambient
  vec3 ambient = ambientSTR * color * light_color; 

  //diffuse
  vec3 diffuse = vec3(0.0,0.0,0.0); 
  //vec3 lightDir = normalize(LightPos[num_light] - FragPos);          
  vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
  if(diffuseSTR > 0.0){
    float diff = max(dot(norm, lightDir),0.0);
    diffuse = diff * (diffuseSTR) * color * light_color;
  }
  //specular
  vec3 specular = vec3(0.0,0.0,0.0);
  if(specularSTR > 0.0){
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



vec2 parallax_mapping_calculation(vec2 texCoords, vec3 final_view_dir){

  vec2 res_tex_coord;  

 // number of depth layers
    const float minLayers = 10;
    const float maxLayers = 20;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), final_view_dir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = final_view_dir.xy / final_view_dir.z * height_scale; // A TEST 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(texture_height1, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(texture_height1, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // -- parallax occlusion mapping interpolation from here on
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(texture_height1, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);


    // set both result
    res_tex_coord = finalTexCoords;
    //final_view_dir = new_viewDir;

    return res_tex_coord;
}



vec3 normal_mapping_calculation(vec2 final_tex_coord)                                                                     
{                                                               
   /* float fact = 1.0;
    
    vec3 Normal = normalize(vsoNormal);                                                       
    vec3 temp_tangent;

    if(var == 1.0){
      temp_tangent = normalize(vec3(-1.0,0.0,0.0));                                                     
    }else{
      temp_tangent = normalize(Tangent);                                                     
    }
    
    temp_tangent = normalize(temp_tangent - dot(temp_tangent, Normal) * Normal);                           
    vec3 Bitemp_tangent = cross(temp_tangent, Normal);                                                
    vec3 BumpMapNormal = texture(texture_normal1, final_tex_coord*fact).xyz;                                
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);                              
    vec3 NewNormal;                                                                         
    mat3 TBN = mat3(temp_tangent, Bitemp_tangent, Normal);                                            
    NewNormal = TBN * BumpMapNormal;                                                        
    NewNormal = normalize(NewNormal);                                                       
    return NewNormal;      */


    vec3 res_normal = vsoNormal;
   
    res_normal = texture(texture_normal1, final_tex_coord).rgb;
  
    res_normal = normalize(res_normal * 2.0 - 1.0);   

    return res_normal;

}




/*vec3 raytrace(vec3 reflectionVector,float startDepth){

  vec3 color = vec3(0.0f);
  
  float size = length(reflectionVector.xy);
  //float size = 10.0;

  //float stepSize = TEST_size; // A TEST  //rayStepSize; 
  float stepSize = size * 0.1; // A TEST  //rayStepSize; 
 

  vec3 _reflectionVector = normalize(reflectionVector/size);
  _reflectionVector = _reflectionVector * stepSize;
        
  // Current sampling position is at current fragment  
  vec2 sampledPosition = (position_for_tex.xy / position_for_tex.w);
  sampledPosition = sampledPosition * 0.5 + 0.5;

  // Current depth at current fragment
  float currentDepth = startDepth;
  
  // The sampled depth at the current sampling position
  //float sampledDepth = texture(texture_depth_SSR, sampledPosition).r; // A TEST
  //float sampledDepth = linearDepth( texture(texture_depth_SSR, sampledPosition).r ); // A TEST


  // Raytrace as long as in texture space of depth buffer (between 0 and 1)
  while(sampledPosition.x <= 1.0 && sampledPosition.x >= 0.0 && sampledPosition.y <= 1.0 && sampledPosition.y >= 0.0){
    // Update sampling position by adding reflection vector's xy and y components
    sampledPosition = sampledPosition + (_reflectionVector.xy * 1.0);
    // Updating depth values
    currentDepth = currentDepth + (_reflectionVector.z * 1.0) * startDepth;
    
    //float sampledDepth = texture(texture_depth_SSR, sampledPosition).r; // A TEST
    float sampledDepth = linearDepth( texture(texture_depth_SSR, sampledPosition).r ); // A TEST


    // If current depth is greater than sampled depth of depth buffer, intersection is found
    if(currentDepth > sampledDepth)
    {
      // Delta is for stop the raytracing after the first intersection is found
      // Not using delta will create "repeating artifacts"
      float delta = (currentDepth - sampledDepth); // A TEST
      //float delta = abs(currentDepth - sampledDepth);
      if(delta < 0.003f )
      {
        color = texture(texture_color_SSR, sampledPosition).rgb;
        break;
      }
    }
  }
 
    //color = vec3(_reflectionVector.x,_reflectionVector.y,_reflectionVector.z);

    return color;
}


vec4 SSR(vec3 _normal){
  
  vec3 reflectedColor = vec3(0.0f);
 
  vec3 normal = _normal; //normalize( texture(deferredNormalTex, vert_UV) ).xyz;
 
  vec2 sampledPosition = (position_for_tex.xy / position_for_tex.w);
  sampledPosition = sampledPosition * 0.5 + 0.5;

  // Depth at current fragment A TEST
  //float currDepth =  texture(texture_depth_SSR, sampledPosition).r;
  float currDepth =  linearDepth( texture(texture_depth_SSR, sampledPosition).r );


  // Eye position, camera is at (0, 0, 0), we look along negative z, add near plane to correct parallax
  vec3 eyePosition = normalize( vec3(viewPos.x, viewPos.y, viewPos.z + camera_near) ); // A TEST
  //vec3 eyePosition = normalize( vec3(0.0, 0.0, camera_near) );
  vec4 reflectionVector = Projection_matrix * reflect( vec4(-eyePosition, 0), vec4(normal, 0) ) ;
 
  // Call raytrace to get reflected color
  reflectedColor = raytrace(reflectionVector.xyz, currDepth); 
 
 
  return vec4(reflectedColor, 1.0f);
}
*/


float distanceSquared(point2 A, point2 B) {
    A -= B;
    return dot(A, A);
}

float reconstructCSZ(float depthBufferValue, vec3 c) {
      return c[0] / (depthBufferValue * c[1] + c[2]);
}


// Returns true if the ray hit something
bool traceScreenSpaceRay1(
 // Camera-space ray origin, which must be within the view volume
 point3 csOrig, 
 
 // Unit length camera-space ray direction
 vec3 csDir,
 
 // A projection matrix that maps to pixel coordinates (not [-1, +1]
 // normalized device coordinates)
 mat4x4 proj, 
 
 // The camera-space Z buffer (all negative values)
 sampler2D csZBuffer,
 
 // Dimensions of csZBuffer
 vec2 csZBufferSize,
 
 // Camera space thickness to ascribe to each pixel in the depth buffer
 float zThickness, 
 
 // (Negative number)
 float nearPlaneZ, 
 
 // Step in horizontal or vertical pixels between samples. This is a float
 // because integer math is slow on GPUs, but should be set to an integer >= 1
 float stride,
 
 // Number between 0 and 1 for how far to bump the ray in stride units
 // to conceal banding artifacts
 float jitter,
 
 // Maximum number of iterations. Higher gives better images but may be slow
 const float maxSteps, 
 
 // Maximum camera-space distance to trace before returning a miss
 float maxDistance, 
 
 // Pixel coordinates of the first intersection with the scene
 out point2 hitPixel, 
 
 // Camera space location of the ray hit
 out point3 hitPoint) {


    // Clip to the near plane    
    float rayLength = ((csOrig.z + csDir.z * maxDistance) > nearPlaneZ) ?
        (nearPlaneZ - csOrig.z) / csDir.z : maxDistance;
    point3 csEndPoint = csOrig + csDir * rayLength;
 
    // Project into homogeneous clip space
    vec4 H0 = proj * vec4(csOrig, 1.0);
    vec4 H1 = proj * vec4(csEndPoint, 1.0);
    float k0 = 1.0 / H0.w, k1 = 1.0 / H1.w;
 
    // The interpolated homogeneous version of the camera-space points  
    point3 Q0 = csOrig * k0, Q1 = csEndPoint * k1;
 
    // Screen-space endpoints
    point2 P0 = H0.xy * k0, P1 = H1.xy * k1;
 
    // If the line is degenerate, make it cover at least one pixel
    // to avoid handling zero-pixel extent as a special case later
    P1 += vec2((distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);
    vec2 delta = P1 - P0;
 
    // Permute so that the primary iteration is in x to collapse
    // all quadrant-specific DDA cases later
    bool permute = false;
    if (abs(delta.x) < abs(delta.y)) { 
        // This is a more-vertical line
        permute = true; delta = delta.yx; P0 = P0.yx; P1 = P1.yx; 
    }
 
    float stepDir = sign(delta.x);
    float invdx = stepDir / delta.x;
 
    // Track the derivatives of Q and k
    vec3  dQ = (Q1 - Q0) * invdx;
    float dk = (k1 - k0) * invdx;
    vec2  dP = vec2(stepDir, delta.y * invdx);
 
    // Scale derivatives by the desired pixel stride and then
    // offset the starting values by the jitter fraction
    dP *= stride; dQ *= stride; dk *= stride;
    P0 += dP * jitter; Q0 += dQ * jitter; k0 += dk * jitter;
 
    // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, k from k0 to k1
    point3 Q = Q0; 
 
    // Adjust end condition for iteration direction
    float  end = P1.x * stepDir;
 
    float k = k0, stepCount = 0.0, prevZMaxEstimate = csOrig.z;
    float rayZMin = prevZMaxEstimate, rayZMax = prevZMaxEstimate;
    float sceneZMax = rayZMax + 100;
    for (point2 P = P0; 
         ((P.x * stepDir) <= end) && (stepCount < maxSteps) &&
         ((rayZMax < sceneZMax - zThickness) || (rayZMin > sceneZMax)) &&
          (sceneZMax != 0); 
         P += dP, Q.z += dQ.z, k += dk, ++stepCount) {
         
        rayZMin = prevZMaxEstimate;
        rayZMax = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
        prevZMaxEstimate = rayZMax;
        if (rayZMin > rayZMax) { 
           float t = rayZMin; rayZMin = rayZMax; rayZMax = t;
        }
 
        hitPixel = permute ? P.yx : P;
        
        hitPixel.y = csZBufferSize.y - hitPixel.y;
        
        // You may need hitPixel.y = csZBufferSize.y - hitPixel.y; here if your vertical axis
        // is different than ours in screen space
        
        //sceneZMax = linearDepth(texelFetch(csZBuffer, ivec2(hitPixel), 0)); // A TEST
        sceneZMax = texelFetch(csZBuffer, ivec2(hitPixel), 0).r; 
        sceneZMax = reconstructCSZ(sceneZMax, clip_info); // A TEST
    
    }
     
    // Advance Q based on the number of steps
    Q.xy += dQ.xy * stepCount;
    hitPoint = Q * (1.0 / k);
    return (rayZMax >= sceneZMax - zThickness) && (rayZMin < sceneZMax);


  //hitPoint = vec3(0.0,1.0,0.0);
  //return true;
  }



void main() {


    vec3 color;
    float final_alpha;
    vec2 final_tex_coord;
    vec3 final_view_dir;

    final_tex_coord = TexCoord;
    //final_view_dir = normalize(viewPos - FragPos);
    final_view_dir = normalize(TangentViewPos - TangentFragPos);

    // get parallax mapping tex coord
    if(var == 0.0 || var == 1.0){

      final_tex_coord = parallax_mapping_calculation(TexCoord, final_view_dir);

     /* if(final_tex_coord.x > 1.0 || final_tex_coord.y > 1.0 || final_tex_coord.x < 0.0 || final_tex_coord.y < 0.0)
        discard;*/
    }

    color = texture(texture_diffuse1, final_tex_coord).rgb;  

    if(var == 1.0 || var == 0.0 && !SSR_pre_rendu){
      //color = texture(texture_depth_SSR, final_tex_coord).rgb;
      
      color = vec3(1.0,1.0,1.0);
    }  


    final_alpha = alpha;


    // LIGHT CALCULATION
    vec3 norm = normalize(vsoNormal);
    // normal mapping
    if(var == 1.0 || var == 0.0){
    
      norm = normal_mapping_calculation(final_tex_coord);
  
    }

    // ADD SSR 
    if(var == 1.0 && !SSR_pre_rendu){
      //color = mix(color,vec3(SSR(norm)),0.5f);
      //color = vec3(SSR(norm));

      //point3 cs_orig = normalize(vec3(0.0, 0.0, camera_near)); // A TEST normalize or not
      //point3 cs_orig = (vec3(0.0, 0.0, camera_near)); // A TEST normalize or not
      //point3 cs_orig = normalize(cs_FragPos);
      point3 cs_orig = cs_FragPos;
      //point3 cs_orig = vec3(cs_FragPos.x,cs_FragPos.y,cs_FragPos.z + camera_near);
      
      
      vec3 cs_dir =(vec3(reflect( vec4(-cs_orig, 0), vec4(norm, 0) ))); // A TEST normalize or not
      //vec3 cs_dir = normalize(vec3(reflect( vec4(-cs_orig, 0), vec4(norm, 0) ))); // A TEST * matrix or not
      //vec3 cs_dir = vec3(projectionMatrix2 * reflect( vec4(-cs_orig, 0), vec4(norm, 0) )); // A TEST * matrix or not
      //vec3 cs_dir = normalize(vec3(projectionMatrix2 * reflect( vec4(-cs_orig, 0), vec4(norm, 0) ))); // A TEST * matrix or not
      

      //mat4x4 proj = projection_Matrix2; // A TEST => la matrix doit map les coordonnées de tex (0.0 à 1.0 ??)
      mat4x4 proj = projectionMatrix2; // A TEST => la matrix doit map les coordonnées de tex (0.0 à 1.0 ??)
      
      vec2 buffer_size = vec2(tex_x_size,tex_y_size);
      float thickness = 1.0; // A TEST 1 2 5 1000
      float nearPlaneZ = camera_near * -1.0; // A TEST (negative value)
      float stride = 1.0;        //
      ivec2 c = ivec2(gl_FragCoord.xy);
      float jitter = 0.0 /*((c.x+c.y)&1)*0.5*/;        //   A
      float max_step = 1.0;     //  TEST
      float max_distance = 1.0; //
      point2 hit_pixel;
      point3 hit_point;

      bool test = traceScreenSpaceRay1(cs_orig, cs_dir, proj, texture_depth_SSR, buffer_size, thickness,
       nearPlaneZ, stride, jitter, max_step, max_distance, hit_pixel, hit_point);

      //color = hit_point;       
      //vec2 sampledPosition = (hit_point.xy);
      vec2 sampledPosition = (hit_pixel);
      sampledPosition = sampledPosition * 0.5 + 0.5;
      //sampledPosition = hit_pixel;

      //if(test)
        //color = texture(texture_color_SSR, sampledPosition).rgb;
    
    }


    LightRes LightRes1 = LightCalculation(0, norm, final_view_dir, color, LightColor[0], LightSpecularColor[0] /*vec3(0.0,0.0,1.0)*/);


    // FINAL LIGHT
    vec3 result = (LightRes1.ambient + LightRes1.diffuse + LightRes1.specular);
   

    // ADD AO mapping
    /*if(){
      vec3 temp_AO = texture(texture_specular1, final_tex_coord).rgb;
      float temp = (temp_AO.r + temp_AO.g + temp_AO.b) / 3.0;
      result *= temp;
      if(var == 2.0)
        result *= temp;
    }*/


    // main out
    FragColor = vec4(result, final_alpha);


    // second out => draw only brighest fragments
    if(!SSR_pre_rendu){
      float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
      if(brightness > 0.7)
        FragColor2 = vec4(result, 1.0);
    }/*else{
      //float depth = depthSample(gl_FragCoord.z);
      float depth = gl_FragCoord.z;
      //float depth = vec3(linearDepth(gl_FragCoord.z));
      gl_FragDepth = gl_FragCoord.z;
      FragColor2 = vec4(vec3(depth), 1.0);
    }*/
  
}
