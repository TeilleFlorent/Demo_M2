#version 330 
#define Point2 vec2
#define Point3 vec3
#define Vector2 vec2
#define Vector3 vec3
#define Vector4 vec4


layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragColor2;

#define MAX_NB_LIGHTS 10
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
in vec3 TangentLightPos[MAX_NB_LIGHTS];
in vec3 TangentViewPos;
in vec3 TangentFragPos;

uniform int nb_lights;
uniform vec3 LightPos[MAX_NB_LIGHTS];
uniform vec3 LightColor[MAX_NB_LIGHTS];
uniform vec3 LightSpecularColor[MAX_NB_LIGHTS];
uniform float constant[MAX_NB_LIGHTS];
uniform float linear[MAX_NB_LIGHTS];
uniform float quadratic[MAX_NB_LIGHTS];

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
uniform mat4 projectionMatrix;
uniform mat4 projectionMatrix2;
uniform mat4 projectionMatrix3;

uniform float height_scale;
uniform bool parallax;
uniform vec2 max_tex_coord;
uniform vec2 min_tex_coord;


uniform sampler2D texture_diffuse1; 
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1; 
uniform sampler2D texture_height1; 
uniform sampler2D texture_AO1; 
uniform sampler2D texture_roughness1; 
uniform sampler2D texture_metalness1; 

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
  vec3 lightDir;
  //if(var == 1.0 /*|| var == 0.0*/){ 
  if(false){ 
  
      lightDir = normalize(TangentLightPos[num_light] - TangentFragPos);

  }else{
    lightDir = normalize(LightPos[num_light] - FragPos);          
  }
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
  float attenuation = 1.0f / (constant[0] + linear[0] * distance + quadratic[0] * (distance * distance)); 


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
    const float minLayers = 5;
    const float maxLayers = 15;
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

  vec3 res_normal;
      
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


float distanceSquared(Point2 A, Point2 B) {
    A -= B;
    return dot(A, A);
}

float reconstructCSZ(float depthBufferValue, vec3 c) {
      return c[0] / (depthBufferValue * c[1] + c[2]);
}


void swap(in out float a, in out float b) {
     float temp = a;
     a = b;
     b = temp;
}


bool traceScreenSpaceRay1
   (Point3          csOrigin, 
    Vector3         csDirection,
    mat4x4          projectToPixelMatrix,
    sampler2D       csZBuffer,
    vec2          csZBufferSize,
    float           csZThickness,
    const in bool   csZBufferIsHyperbolic,
    vec3          clipInfo,
    float           nearPlaneZ,
    float     stride,
    float           jitterFraction,
    float           maxSteps,
    in float        maxRayTraceDistance,
    out Point2      hitPixel,
    out int         hitLayer,
  out Point3    csHitPoint
//    ,out Color3      debugColor
    ) {
    vec3 debugColor = vec3(0.0);
    // Clip ray to a near plane in 3D (doesn't have to be *the* near plane, although that would be a good idea)
    float rayLength = ((csOrigin.z + csDirection.z * maxRayTraceDistance) > nearPlaneZ) ?
                        (nearPlaneZ - csOrigin.z) / csDirection.z :
                        maxRayTraceDistance;
  Point3 csEndPoint = csDirection * rayLength + csOrigin;

    // Project into screen space
    Vector4 H0 = projectToPixelMatrix * Vector4(csOrigin, 1.0);
    Vector4 H1 = projectToPixelMatrix * Vector4(csEndPoint, 1.0);

    // There are a lot of divisions by w that can be turned into multiplications
    // at some minor precision loss...and we need to interpolate these 1/w values
    // anyway.
    //
    // Because the caller was required to clip to the near plane,
    // this homogeneous division (projecting from 4D to 2D) is guaranteed 
    // to succeed. 
    float k0 = 1.0 / H0.w;
    float k1 = 1.0 / H1.w;

    // Switch the original points to values that interpolate linearly in 2D
    Point3 Q0 = csOrigin * k0; 
    Point3 Q1 = csEndPoint * k1;

  // Screen-space endpoints
    Point2 P0 = H0.xy * k0;
    Point2 P1 = H1.xy * k1;

    // [Optional clipping to frustum sides here]

    // Initialize to off screen
    hitPixel = Point2(-1.0, -1.0);
    hitLayer = 0; // Only one layer

    // If the line is degenerate, make it cover at least one pixel
    // to avoid handling zero-pixel extent as a special case later
    P1 += Vector2((distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);

    Vector2 delta = P1 - P0;

    // Permute so that the primary iteration is in x to reduce
    // large branches later
    bool permute = (abs(delta.x) < abs(delta.y));
  if (permute) {
    // More-vertical line. Create a permutation that swaps x and y in the output
        // by directly swizzling the inputs.
    delta = delta.yx;
    P1 = P1.yx;
    P0 = P0.yx;        
  }
    
  // From now on, "x" is the primary iteration direction and "y" is the secondary one
    float stepDirection = sign(delta.x);
    float invdx = stepDirection / delta.x;
    Vector2 dP = Vector2(stepDirection, invdx * delta.y);

    // Track the derivatives of Q and k
    Vector3 dQ = (Q1 - Q0) * invdx;
    float   dk = (k1 - k0) * invdx;
    
    // Because we test 1/2 a texel forward along the ray, on the very last iteration
    // the interpolation can go past the end of the ray. Use these bounds to clamp it.
    float zMin = min(csEndPoint.z, csOrigin.z);
    float zMax = max(csEndPoint.z, csOrigin.z);

    // Scale derivatives by the desired pixel stride
  dP *= stride; dQ *= stride; dk *= stride;

    // Offset the starting values by the jitter fraction
  P0 += dP * jitterFraction; Q0 += dQ * jitterFraction; k0 += dk * jitterFraction;

  // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, and k from k0 to k1
    Point3 Q = Q0;
    float  k = k0;

  // We track the ray depth at +/- 1/2 pixel to treat pixels as clip-space solid 
  // voxels. Because the depth at -1/2 for a given pixel will be the same as at 
  // +1/2 for the previous iteration, we actually only have to compute one value 
  // per iteration.
  float prevZMaxEstimate = csOrigin.z;
    float stepCount = 0.0;
    float rayZMax = prevZMaxEstimate, rayZMin = prevZMaxEstimate;
    float sceneZMax = rayZMax + 1e4;

    // P1.x is never modified after this point, so pre-scale it by 
    // the step direction for a signed comparison
    float end = P1.x * stepDirection;

    // We only advance the z field of Q in the inner loop, since
    // Q.xy is never used until after the loop terminates.

    Point2 P;
  for (P = P0;
        ((P.x * stepDirection) <= end) && 
        (stepCount < maxSteps) &&
        ((rayZMax < sceneZMax - csZThickness) ||
            (rayZMin > sceneZMax)) &&
        (sceneZMax != 0.0);
        P += dP, Q.z += dQ.z, k += dk, stepCount += 1.0) {

        // The depth range that the ray covers within this loop
        // iteration.  Assume that the ray is moving in increasing z
        // and swap if backwards.  Because one end of the interval is
        // shared between adjacent iterations, we track the previous
        // value and then swap as needed to ensure correct ordering
        rayZMin = prevZMaxEstimate;

        // Compute the value at 1/2 step into the future
        rayZMax = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
        rayZMax = clamp(rayZMax, zMin, zMax);
    prevZMaxEstimate = rayZMax;

        // Since we don't know if the ray is stepping forward or backward in depth,
        // maybe swap. Note that we preserve our original z "max" estimate first.
        if (rayZMin > rayZMax) { swap(rayZMin, rayZMax); }

        // Camera-space z of the background
        hitPixel = permute ? P.yx : P;
        sceneZMax = texelFetch(csZBuffer, ivec2(hitPixel), 0).r;

        // This compiles away when csZBufferIsHyperbolic = false
        if (csZBufferIsHyperbolic) {
            sceneZMax = reconstructCSZ(sceneZMax, clipInfo);
        }
    } // pixel on ray

    // Undo the last increment, which ran after the test variables
    // were set up.
    P -= dP; Q.z -= dQ.z; k -= dk; stepCount -= 1.0;

    bool hit = (rayZMax >= sceneZMax - csZThickness) && (rayZMin <= sceneZMax);

    // If using non-unit stride and we hit a depth surface...
    if ((stride > 1) && hit) {
        // Refine the hit point within the last large-stride step
        
        // Retreat one whole stride step from the previous loop so that
        // we can re-run that iteration at finer scale
        P -= dP; Q.z -= dQ.z; k -= dk; stepCount -= 1.0;

        // Take the derivatives back to single-pixel stride
        float invStride = 1.0 / stride;
        dP *= invStride; dQ.z *= invStride; dk *= invStride;

        // For this test, we don't bother checking thickness or passing the end, since we KNOW there will
        // be a hit point. As soon as
        // the ray passes behind an object, call it a hit. Advance (stride + 1) steps to fully check this 
        // interval (we could skip the very first iteration, but then we'd need identical code to prime the loop)
        float refinementStepCount = 0;

        // This is the current sample point's z-value, taken back to camera space
        prevZMaxEstimate = Q.z / k;
        rayZMin = prevZMaxEstimate;

        // Ensure that the FOR-loop test passes on the first iteration since we
        // won't have a valid value of sceneZMax to test.
        sceneZMax = rayZMin - 1e7;

        for (;
            (refinementStepCount <= stride*1.4) &&
            (rayZMin > sceneZMax) && (sceneZMax != 0.0);
            P += dP, Q.z += dQ.z, k += dk, refinementStepCount += 1.0) {

            rayZMin = prevZMaxEstimate;

            // Compute the ray camera-space Z value at 1/2 fine step (pixel) into the future
            rayZMax = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
            rayZMax = clamp(rayZMax, zMin, zMax);

            prevZMaxEstimate = rayZMax;
            rayZMin = min(rayZMax, rayZMin);

            hitPixel = permute ? P.yx : P;
            //hitPixel.y = csZBufferSize.y - hitPixel.y; 

            sceneZMax = texelFetch(csZBuffer, ivec2(hitPixel), 0).r;

            if (csZBufferIsHyperbolic) {
                sceneZMax = reconstructCSZ(sceneZMax, clipInfo);
            }
        }

        // Undo the last increment, which happened after the test variables were set up
        Q.z -= dQ.z; refinementStepCount -= 1;

        // Count the refinement steps as fractions of the original stride. Save a register
        // by not retaining invStride until here
        stepCount += refinementStepCount / stride;
      //  debugColor = vec3(refinementStepCount / stride);
    } // refinement

    Q.xy += dQ.xy * stepCount;
    csHitPoint = Q * (1.0 / k);

    // Support debugging. This will compile away if debugColor is unused
    if ((P.x * stepDirection) > end) {
        // Hit the max ray distance -> blue
        debugColor = vec3(0,0,1);
    } else if (stepCount >= maxSteps) {
        // Ran out of steps -> red
        debugColor = vec3(1,0,0);
    } else if (sceneZMax == 0.0) {
        // Went off screen -> yellow
        debugColor = vec3(1,1,0);
    } else {
        // Encountered a valid hit -> green
        // ((rayZMax >= sceneZMax - csZThickness) && (rayZMin <= sceneZMax))
        debugColor = vec3(0,1,0);
    }
        
    // Does the last point discovered represent a valid hit?
    return hit;
}

// PBR FUNCTIONS
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}





void main() {


    vec3 albedo, result;
    float final_alpha;
    vec2 final_tex_coord;
    vec3 final_view_dir;

    final_tex_coord = TexCoord;
    final_view_dir = normalize(viewPos - FragPos);
    

    // get parallax mapping tex coord
    if((var == 1.0 || var == 0.0)){
    //if(false){

      final_view_dir = normalize(TangentViewPos - TangentFragPos);

      if(parallax == true)
        final_tex_coord = parallax_mapping_calculation(TexCoord, final_view_dir);

      if(var == 1.0){
        if(final_tex_coord.x > 1.0 * 5.0 || final_tex_coord.y > 1.0 * 5.0 || final_tex_coord.x < 0.0 || final_tex_coord.y < 0.0)
          discard;
      }else{
      /*  //vec2 max = vec2(3.0,2.0);
        //vec2 min = vec2(-1.0,-1.0);
        vec2 max = vec2(3.0,2.0);
        vec2 min = vec2(0.0,0.0);
        
        //max = normalize(max);
        //min = normalize(min);
        //if(final_tex_coord.x >= max.x || final_tex_coord.y >= max.y  || final_tex_coord.x <= min.x || final_tex_coord.y <= min.y)
          //discard;*/
      }
    
    }

    albedo = texture(texture_diffuse1, final_tex_coord).rgb;  
    final_alpha = alpha;
    result = vec3(0.0);

    if(var == 1.0){
      //albedo = texture(texture_metalness1, final_tex_coord).rgb;
      
      //color = vec3(0.3);
    }  



    // NORMAL CALCULATION
    vec3 norm = normalize(vsoNormal);
    // normal mapping
    if(var == 1.0 || var == 0.0){
    
      norm = normal_mapping_calculation(final_tex_coord);
  
    }


    // ADD SSR 
    if(var == 1.0 && !SSR_pre_rendu){
      
      //Point3 cs_orig = normalize(vec3(0.0, 0.0, camera_near)); // A TEST normalize or not
        Point3 cs_orig = normalize(vec3(viewPos.x, viewPos.y, viewPos.z + camera_near)); // A TEST normalize or not
      //Point3 cs_orig = (vec3(0.0, 0.0, 0.0)); // A TEST normalize or not
      //Point3 cs_orig = normalize(cs_FragPos);
      //Point3 cs_orig = cs_FragPos;
      //Point3 cs_orig = vec3(cs_FragPos.x,cs_FragPos.y,cs_FragPos.z + camera_near);
      //Point3 cs_orig = normalize(vec3(cs_FragPos.x,cs_FragPos.y,cs_FragPos.z + camera_near));
      
      
      //vec3 cs_dir =(vec3(reflect( vec4(-cs_orig, 0), vec4(norm, 0) ))); // A TEST normalize or not
      //vec3 cs_dir = normalize(vec3(reflect( vec4(-cs_orig, 0), vec4(norm, 0) ))); // A TEST * matrix or not
      //vec3 cs_dir = vec3( reflect( vec4(-cs_orig, 0), vec4(norm, 0) )); // A TEST * matrix or not
      //vec3 cs_dir = normalize(vec3(projectionMatrix2 * reflect( vec4(-cs_orig, 0), vec4(norm, 0) ))); // A TEST * matrix or not
      vec3 cs_dir = normalize(vec3(vec4((viewPos - cs_FragPos),0)));  


      //mat4x4 proj = projectionMatrix2 * projectionMatrix3; // A TEST => la matrix doit map les coordonnées de tex (0.0 à 1.0 ??)
      //mat4x4 proj = projectionMatrix3 * projectionMatrix2; // A TEST => la matrix doit map les coordonnées de tex (0.0 à 1.0 ??)
      mat4x4 proj = projectionMatrix; // A TEST => la matrix doit map les coordonnées de tex (0.0 à 1.0 ??)
      

      vec2 buffer_size = vec2(tex_x_size,tex_y_size);
      float thickness = 1.0; // A TEST 1 2 5 1000
      float nearPlaneZ = camera_near * -1.0; // A TEST (negative value)
      float stride = 1.0;        //
      ivec2 c = ivec2(gl_FragCoord.xy);
      float jitter = 0.001 /*((c.x+c.y)&1)*0.5*/;        //   A
      //float max_step = 1.0;     //  TEST
      float max_step = 15.0;     //  TEST
      
      //float max_distance = 1.0; //
      float max_distance = 200.0; //
      
      Point2 hit_pixel;
      Point3 hit_point;
      int layer;

      //bool test = traceScreenSpaceRay1(cs_orig, cs_dir, proj, texture_depth_SSR, buffer_size, thickness, true /*false*/, clip_info,
       //nearPlaneZ, stride, jitter, max_step, max_distance, hit_pixel, layer , hit_point);

      //color = hit_point;       
      //vec2 sampledPosition = (hit_point.xy);
      vec2 sampledPosition = (hit_pixel);
      //sampledPosition = sampledPosition * 0.5 + 0.5; 
      //sampledPosition = hit_pixel;

      //if(test)
        //color = texture(texture_color_SSR, sampledPosition).rgb;
    
    }

    // BLIN PHONG LIGHT CALCULATION
    /*LightRes LightRes1 = LightCalculation(0, norm, final_view_dir, albedo, LightColor[0], LightSpecularColor[0]);
    LightRes LightRes2 = LightCalculation(1, norm, final_view_dir, albedo, LightColor[1], LightSpecularColor[1]);
  
    result = (LightRes1.ambient + LightRes1.diffuse + LightRes1.specular);
    result += (LightRes2.diffuse + LightRes2.specular);*/
    

    // PBR LIGHT CALCULATION
    vec3 R = reflect(-final_view_dir, norm);
    albedo =  pow(texture(texture_diffuse1, final_tex_coord).rgb, vec3(2.2));
    float metalness = texture(texture_metalness1, final_tex_coord).r;
    float roughness = texture(texture_roughness1, final_tex_coord).r;
    float ao        = texture(texture_AO1, final_tex_coord).r; 
    if(var == 1.0){
      //ao = 1.0;
    }

     // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use their albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metalness);  

     vec3 Lo = vec3(0.0);
    for(int i = 0; i < nb_lights; ++i) 
    {
        // calculate per-light radiance
        vec3 L;
        if(var == 0 || var == 1){
          L = normalize(TangentLightPos[i] - TangentFragPos);
        }else{
          L = normalize(LightPos[i] - FragPos);
        }

        vec3 H = normalize(final_view_dir + L);
        
        float distance;
        if(var == 0 || var == 1){
          distance = length(TangentLightPos[i] - TangentFragPos);
        }else{
          distance = length(LightPos[i] - FragPos);
        }

        
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = (LightColor[i] * 100.0) * attenuation; // * 100 pour avoir le meme ordre de grandeur que dans l'exemple

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(norm, H, roughness);   
        float G   = GeometrySmith(norm, final_view_dir, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, final_view_dir), 0.0), F0);
           
        vec3 nominator    = NDF * G * F; 
        float denominator = 4 * max(dot(final_view_dir, norm), 0.0) * max(dot(L, norm), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 brdf = nominator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metalness;   

        // scale light by NdotL
        float NdotL = max(dot(norm, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + brdf) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    vec3 ambient = ambientSTR * albedo * ao;
    result = ambient + Lo;
   
    // HDR tonemapping
    //result = result / (result + vec3(1.0));
    // gamma correct
    //result = pow(result, vec3(1.0/2.2)); 



    //if(var == 1.0)
      //result = texture(texture_roughness1, final_tex_coord).rgb;
    // main out
    FragColor = vec4(result, final_alpha);


    // second out => draw only brighest fragments
    if(!SSR_pre_rendu){
      float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
      if(brightness > 0.99)
        FragColor2 = vec4(result, 1.0);
    }
    /*else{
      //float depth = depthSample(gl_FragCoord.z);
      float depth = gl_FragCoord.z;
      //float depth = vec3(linearDepth(gl_FragCoord.z));
      gl_FragDepth = gl_FragCoord.z;
      FragColor2 = vec4(vec3(depth), 1.0);
    }*/
  
}
