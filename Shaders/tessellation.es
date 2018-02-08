#version 410 core


//******************************************************************************
//**********  Evaluation shader inputs/ouputs  *********************************
//******************************************************************************


// Input layout, define the configuration of the input patch from the tessellation control shader
// ----------------------------------------------------------------------------------------------
layout( triangles, equal_spacing, ccw ) in;


// Evaluation shader input uniforms
// --------------------------------
uniform mat4      uProjectionMatrix;
uniform mat4      uViewMatrix;             
uniform sampler2D uTextureHeight1; 
uniform float 		uDisplacementFactor;


// Attributes of the input Control Points from the Control shader                                                                  
// --------------------------------------------------------------    
in vec3 oFragPosToES[];                                                                      
in vec3 oNormalToES[];      
in vec2 oUVToES[];                                                                      


// Attributes of the output vertex to the fragment shader                                                               
// ------------------------------------------------------
out vec3 oFragPos;
out vec3 oNormal;
out vec2 oUV;


//******************************************************************************
//**********  Evaluation shader functions  *************************************
//******************************************************************************


// Interpolation functions using barycentric coordinates from the primitive generator
// ----------------------------------------------------------------------------------
vec2 InterpolateVec2( vec2 iVector0,
										  vec2 iVector1,
										  vec2 iVector2 )                                                   
{                                                                                               
	return ( vec2( gl_TessCoord.x ) * iVector0 ) 
	     + ( vec2( gl_TessCoord.y ) * iVector1 ) 
	     + ( vec2( gl_TessCoord.z ) * iVector2 );   
}                                                                                               
                                                                                                
vec3 InterpolateVec3( vec3 iVector0,
										  vec3 iVector1,
										  vec3 iVector2 )                                                   
{                                                                                               
	return ( vec3( gl_TessCoord.x ) * iVector0 ) 
	     + ( vec3( gl_TessCoord.y ) * iVector1 ) 
	     + ( vec3( gl_TessCoord.z ) * iVector2 );   
}     


// Main function
// -------------
void main()                                                                                     
{
	// Interpolate the attributes of the output vertex using the barycentric coordinates    
	oFragPos = InterpolateVec3( oFragPosToES[ 0 ],
														  oFragPosToES[ 1 ], 
														  oFragPosToES[ 2 ] );   

	oNormal = InterpolateVec3( oNormalToES[ 0 ],
													   oNormalToES[ 1 ],
													   oNormalToES[ 2 ] );            
	oNormal = normalize( oNormal );

	oUV = InterpolateVec2( oUVToES[ 0 ],
												 oUVToES[ 1 ],
												 oUVToES[ 2 ] );    

	// Perform the displacement mapping of the tessellate vertex along the normal
	float displacement = texture( uTextureHeight1, oUV ).r;
  oFragPos += oNormal * displacement * uDisplacementFactor;
  gl_Position = uProjectionMatrix * uViewMatrix * vec4( oFragPos, 1.0 );                                              
}

