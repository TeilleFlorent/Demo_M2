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
}; 


//******************************************************************************
//**********  Control shader inputs/ouputs  ************************************
//******************************************************************************


// Output layout, define the number of Control Points in the output patch
// ---------------------------------------------------------------------                                                 
layout ( vertices = 1 ) out;


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
out patch OutputPatch oPatch;                                                                       


//******************************************************************************
//**********  Control shader functions  ****************************************
//******************************************************************************


// Function used to generate control midpoint define by th nearest vertex and its normal
// -------------------------------------------------------------------------------------
vec3 ProjectToPlane( vec3 iPoint, 
										 vec3 iPlanePoint,
										 vec3 iPlaneNormal )                              
{                                                                                               
	vec3 v = iPoint - iPlanePoint;                                                                
	float length = dot( v, iPlaneNormal );                                                            
	
	vec3 distance = length * iPlaneNormal;                                                                 
	
	// Return projected point
	return ( iPoint - distance );                                                                         
}                                                                                               
 

// Function used to generate the position of the 10 output control points
// ----------------------------------------------------------------------

//
//      B003-------B012-------B021------------B030
//          |      /\         / \            /
//          |    /   \      /    \         /
//          |  /      \   /       \      /
//          |/         \/          \   /
//        B102-------B111-----------B120
//          |         /\             / 
//          |       /   \          /  
//          |     /      \       /   
//          |   /         \    /    
//          | /            \ /
//        B201------------B210
//          |            /
//          |    		   /
//					|	       /
//          |      /
//					|	   /
//          |  /
//          |/
//         B300

void GenControlPointsPosition()
{
	// The original control points position stay the same
	oPatch._frag_pos_B030 = oFragPosToCS[ 0 ];
	oPatch._frag_pos_B003 = oFragPosToCS[ 1 ];
	oPatch._frag_pos_B300 = oFragPosToCS[ 2 ];

	// Edges are names according to the opposing vertex
	vec3 EdgeB300 = oPatch._frag_pos_B003 - oPatch._frag_pos_B030;
	vec3 EdgeB030 = oPatch._frag_pos_B300 - oPatch._frag_pos_B003;
	vec3 EdgeB003 = oPatch._frag_pos_B030 - oPatch._frag_pos_B300;

	// Generate two midpoints on each edge
	oPatch._frag_pos_B021 = oPatch._frag_pos_B030 + ( EdgeB300 / 3.0 );
	oPatch._frag_pos_B012 = oPatch._frag_pos_B030 + ( ( EdgeB300 / 3.0 ) * 2.0 );
	oPatch._frag_pos_B102 = oPatch._frag_pos_B003 + ( EdgeB030 / 3.0 );
	oPatch._frag_pos_B201 = oPatch._frag_pos_B003 + ( ( EdgeB030 / 3.0 ) *2.0 );
	oPatch._frag_pos_B210 = oPatch._frag_pos_B300 + ( EdgeB003 / 3.0 );
	oPatch._frag_pos_B120 = oPatch._frag_pos_B300 + ( ( EdgeB003 / 3.0 ) * 2.0 );

	// Project each midpoint on the plane defined by the nearest vertex and its normal
  oPatch._frag_pos_B021 = ProjectToPlane( oPatch._frag_pos_B021, 
  																			 oPatch._frag_pos_B030,
                                         oPatch._normal[ 0 ] );
  oPatch._frag_pos_B012 = ProjectToPlane( oPatch._frag_pos_B012, 
  																			 oPatch._frag_pos_B003,
                                         oPatch._normal[ 1 ] );
  oPatch._frag_pos_B102 = ProjectToPlane( oPatch._frag_pos_B102,
  																			 oPatch._frag_pos_B003,
                                         oPatch._normal[ 1 ] );
  oPatch._frag_pos_B201 = ProjectToPlane( oPatch._frag_pos_B201,
  																			 oPatch._frag_pos_B300,
                                         oPatch._normal[ 2 ] );
  oPatch._frag_pos_B210 = ProjectToPlane( oPatch._frag_pos_B210,
  																		   oPatch._frag_pos_B300,
                                         oPatch._normal[ 2 ] );
  oPatch._frag_pos_B120 = ProjectToPlane( oPatch._frag_pos_B120,
  																			 oPatch._frag_pos_B030,
                                         oPatch._normal[ 0 ] );

  // 3 vertice average
  vec3 Center = ( oPatch._frag_pos_B003 + oPatch._frag_pos_B030 + oPatch._frag_pos_B300 ) / 3.0;  

  // 6 control point average
  oPatch._frag_pos_B111 = ( oPatch._frag_pos_B021 + oPatch._frag_pos_B012 + oPatch._frag_pos_B102 +             
                            oPatch._frag_pos_B201 + oPatch._frag_pos_B210 + oPatch._frag_pos_B120) / 6.0; 

  // In order to calculate the position of B111 we take a vector from the original 
  // triangle center ( average of the three vertices ) to the average of the 6 midpoints ( after projection ).
  // We continue along that vector for one half of its length.
  oPatch._frag_pos_B111 += ( oPatch._frag_pos_B111 - Center ) / 2.0;    
}


// Main function
// -------------
void main()                                                                                     
{                                                                                        
	// Set normal and UV of the 3 "principal" control points of the output patch
	for( int it = 0 ; it < 3 ; it++ )
	{
		oPatch._normal[ it ] = oNormalToCS[ it ];
		oPatch._uv[ it ]     = oUVToCS[ it ];
	}

	// Set all control points position 
	GenControlPointsPosition();

	// Calculate the tessellation levels, corresponding to the segments count of each triangle edge                                                       
	gl_TessLevelOuter[ 0 ] = 2;                                               
	gl_TessLevelOuter[ 1 ] = 2;                                                  
	gl_TessLevelOuter[ 2 ] = 2;                                                  
	gl_TessLevelInner[ 0 ] = 1;                                                 	                           
}   
