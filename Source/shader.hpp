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

    void SetShaderClassicPipeline( const GLchar * iVertexPath,
                                   const GLchar * iFragmentPath );
    
    void SetShaderGeometryPipeline( const GLchar * iVertexPath,
                                    const GLchar * iGeometryPath,
                                    const GLchar * iFragmentPath );
    
    GLuint _program;
    
};

#endif  // SHADER_H

