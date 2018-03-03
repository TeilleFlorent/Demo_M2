#version 330 core

#define PI 3.14159265358979323846264338
#define ZERO 0.00390625


//******************************************************************************
//**********  Fragment shader inputs/ouputs  ***********************************
//******************************************************************************


// Fragment color output(s)
// ------------------------
layout ( location = 0 ) out vec4 FragColor;


// Fragment input uniforms
// -----------------------
uniform samplerCube uEnvironmentMap;
uniform float uSampleDelta;

// Fragment inputs from vertex shader 
// ----------------------------------
in vec3 oWorldPos;


//******************************************************************************
//**********  Fragment shader functions  ***************************************
//******************************************************************************

void main()
{		
  vec3 normal = normalize( oWorldPos );

  vec3 irradiance = vec3( 0.0 );   
  
  // Tangent space calculation from origin point
  vec3 up    = vec3( 0.0, 1.0, 0.0 );
  vec3 right = cross( up, normal );
  up         = cross( normal, right );
     
  float sample_count = 0.0f;
  for( float phi = 0.0; phi < 2.0 * PI; phi += uSampleDelta )
  {
    for( float theta = 0.0; theta < 0.5 * PI; theta += uSampleDelta )
    {
      // Spherical to cartesian (in tangent space)
      vec3 tangent_sample = vec3( sin( theta ) * cos( phi ), sin( theta ) * sin( phi ), cos( theta ) );
      
      // Tangent space to world
      vec3 sample_vector = ( tangent_sample.x * right ) + ( tangent_sample.y * up ) + ( tangent_sample.z * normal ); 

      vec3 sample_color = texture( uEnvironmentMap, sample_vector ).rgb;
      if( length( sample_color ) > 0.0 )
      { 
        irradiance += sample_color * cos( theta ) * sin( theta );
        sample_count++;
      }
    }
  }

  irradiance = PI * irradiance * ( 1.0 / sample_count );
  FragColor = vec4( irradiance , 1.0 );
}
