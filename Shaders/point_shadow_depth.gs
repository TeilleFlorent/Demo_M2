#version 410 core


//******************************************************************************
//**********  Geometry shader inputs/ouputs  ***********************************
//******************************************************************************


// Geometry input/output layout
// ----------------------------
layout ( triangles ) in;
layout ( triangle_strip, max_vertices = 18 ) out;


// Geometry shader input uniform(s)
// --------------------------------
uniform mat4 uShadowTransformMatrices[ 6 ];


// Geometry shader outputs to fragment shader	
// ------------------------------------------
out vec4 oFragPos;


//******************************************************************************
//**********  Geomtry shader function(s)  **************************************
//******************************************************************************

void main()
{
	for( int face_it = 0; face_it < 6; face_it )
	{	
		// Specify which face we are rendering using this built-in variable
		gl_Layer = face_it; 

		// For each triangle's vertices
		for( int i = 0; i < 3; i++ ) 
		{
		  oFragPos = gl_in[i].gl_Position;
		  gl_Position = uShadowTransformMatrices[ face_it ] * oFragPos;
		  EmitVertex();
		}    
		EndPrimitive();
	}
} 