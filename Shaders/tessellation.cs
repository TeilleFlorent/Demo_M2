#version 410 core


//******************************************************************************
//**********  Control shader inputs/ouputs  ************************************
//******************************************************************************


// Output layout, define the number of Control Points in the output patch
// ---------------------------------------------------------------------                                                 
layout ( vertices = 3 ) out;


// Control shader input uniforms
// -----------------------------
uniform vec3 uViewPosition;       


// Attributes of the input Control Points from the vertex shader                                                                  
// -------------------------------------------------------------
in vec3 oFragPosToCS[];                                                                       
in vec2 oUVToCS[];                                                                       
in vec3 oNormalToCS[];    


// Attributes of the output Control Points send to the tessellation evaluation shader                                                               
// ----------------------------------------------------------------------------------
out vec3 oFragPosToES[];                                                                      
out vec2 oUVToES[];                                                                      
out vec3 oNormalToES[];                                                                     


//******************************************************************************
//**********  Control shader functions  ****************************************
//******************************************************************************


// Function used to get tesselation level from the distance of the two vertices of the edge
// ----------------------------------------------------------------------------------------
float GetTessellationLevel( float iDistance0,
													  float iDistance1 )                                            
{         
	return 300.0;

	float average_distance = ( iDistance0 + iDistance1 ) / 2.0;                                          
	                                                                                            
	if( average_distance <= 6.0 )
	{                                                                   
		return 1000.0;                                                                            
	}                                                                                           
	else
	if( average_distance <= 10.0 )
	{                                                              
		return 750;                                                                             
	}                                                                                           
	else 
	{                                                                                      
		return 300.0;                                                                             
	}                                                                                           
}   


// Main function
// -------------
void main()                                                                                     
{                                                                                               
	// Set the control points of the output patch, copy of the input control point, don't need to transform them                                               
	oFragPosToES[ gl_InvocationID ] = oFragPosToCS[ gl_InvocationID ];                          
	oNormalToES[  gl_InvocationID ] = oNormalToCS[  gl_InvocationID ];                            
	oUVToES[ 			gl_InvocationID ]	= oUVToCS[ 			gl_InvocationID ];                          
	                                                                                            
	// Calculate the distance from the view position to the three control points                       
	float ViewToVertexDistance0 = distance( uViewPosition,
																					oFragPosToES[ 0 ] );                     
	float ViewToVertexDistance1 = distance( uViewPosition,
																					oFragPosToES[ 1 ] );                     
	float ViewToVertexDistance2 = distance( uViewPosition,
																			    oFragPosToES[ 2 ] );

	// Calculate the tessellation levels, corresponding to the segments count of each triangle edge                                                       
	gl_TessLevelOuter[ 0 ] = GetTessellationLevel( ViewToVertexDistance1,
																						     ViewToVertexDistance2 );            
	gl_TessLevelOuter[ 1 ] = GetTessellationLevel( ViewToVertexDistance2, 
																								 ViewToVertexDistance0 );            
	gl_TessLevelOuter[ 2 ] = GetTessellationLevel( ViewToVertexDistance0,
																								 ViewToVertexDistance1 );            
	
	// Inner level correspond to the inner triangle ring count
	gl_TessLevelInner[ 0 ] = gl_TessLevelOuter[ 2 ];                                                
}   
