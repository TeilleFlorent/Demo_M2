#version 410 core


// Output patch structure
struct OutputPatch                                                                              
{                                                                                               
	vec3 _frag_pos_B030;                                                                         
	vec3 _frag_pos_B021;                                                                         
	vec3 _frag_pos_B012;                                                                         
	vec3 _frag_pos_B003;                                                                         
	vec3 _frag_pos_B102;                                                                         
	vec3 _frag_pos_B201;                                                                         
	vec3 _frag_pos_B300;                                                                         
	vec3 _frag_pos_B210;                                                                         
	vec3 _frag_pos_B120;                                                                         
	vec3 _frag_pos_B111;                                                                         
	vec3 _normal[ 3 ];                                                                             
	vec2 _uv[ 3 ];         
	vec3 _tangent[ 3 ];
	vec3 _bitangent[ 3 ];                                                                  
}; 


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
in patch OutputPatch oPatch;                                                                    


// Attributes of the output vertex to the fragment shader                                                               
// ------------------------------------------------------
out vec3 oFragPos;
out vec3 oNormal;
out vec2 oUV;
out vec3 oTBN[ 3 ];


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
	// Get normal from interpolation
	oNormal = InterpolateVec3( oPatch._normal[ 0 ],
													   oPatch._normal[ 1 ],
													   oPatch._normal[ 2 ] );            

	// Get UV from interpolation
	oUV = InterpolateVec2( oPatch._uv[ 0 ],
												 oPatch._uv[ 1 ],
												 oPatch._uv[ 2 ] );

	// Get tangent from interpolation
	vec3 tangent = InterpolateVec3( oPatch._tangent[ 0 ],
															    oPatch._tangent[ 1 ],
															    oPatch._tangent[ 2 ] );

	// Get bitangent from interpolation
	vec3 bitangent = InterpolateVec3( oPatch._bitangent[ 0 ],
															      oPatch._bitangent[ 1 ],
															      oPatch._bitangent[ 2 ] );

	// TBN matrix calculation
	mat3 TBN = transpose( mat3( tangent, bitangent, oNormal ) );
	oTBN[ 0 ] = TBN[ 0 ];
	oTBN[ 1 ] = TBN[ 1 ];
	oTBN[ 2 ] = TBN[ 2 ];

	// Get barycentric coordinates of the tessellated vertice
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	// Use bezier triangle equation with barycentric coordinates to calculate FragPos ( world space position )
	float uPow3 = pow( u, 3 );
	float vPow3 = pow( v, 3 );
	float wPow3 = pow( w, 3 );
	float uPow2 = pow( u, 2 );
	float vPow2 = pow( v, 2 );
	float wPow2 = pow( w, 2 );
	oFragPos = oPatch._frag_pos_B300 * wPow3 +
             oPatch._frag_pos_B030 * uPow3 +
             oPatch._frag_pos_B003 * vPow3 +
             oPatch._frag_pos_B210 * 3.0 * wPow2 * u +
             oPatch._frag_pos_B120 * 3.0 * w * uPow2 +
             oPatch._frag_pos_B201 * 3.0 * wPow2 * v +
             oPatch._frag_pos_B021 * 3.0 * uPow2 * v +
             oPatch._frag_pos_B102 * 3.0 * w * vPow2 +
             oPatch._frag_pos_B012 * 3.0 * u * vPow2 +
             oPatch._frag_pos_B111 * 6.0 * w * u * v;

	// Perform the displacement mapping of the tessellate vertex along the normal
	float displacement = texture( uTextureHeight1, oUV ).r;
  oFragPos += oNormal * displacement * uDisplacementFactor;

  // final vertex output
  gl_Position = uProjectionMatrix * uViewMatrix * vec4( oFragPos, 1.0 );                                              
}

