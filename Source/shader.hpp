#ifndef SHADER_H
#define SHADER_H

#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

#define GLEW_STATIC
#include <GL/glew.h>


//******************************************************************************
//**********  Class Shader  ****************************************************
//******************************************************************************

class Shader
{
  public:

    Shader();

    void Use();

    void SetShaderClassicPipeline( const char * iVertexPath,
                                   const char * iFragmentPath );
    
    void SetShaderGeometryPipeline( const char * iVertexPath,
                                    const char * iGeometryPath,
                                    const char * iFragmentPath );
    
    unsigned int _program;
    
};

#endif  // SHADER_H

