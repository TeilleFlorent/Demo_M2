#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


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
