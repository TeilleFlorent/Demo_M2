#include <SDL2/SDL.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include "camera.hpp"

using namespace std;


//******************************************************************************
//**********  Class Window  ****************************************************
//******************************************************************************

class Window
{

  public:


    // Window functions
    // ----------------

    Window();

    void Initialization();  

    void InitGL( SDL_Window * iWindow );

    SDL_Window * InitSDLWindow( int iWidth,
                                int iHeight,
                                SDL_GLContext * iOpenGLContext );

    void Resize();

    void ManageEvents( Camera * iCamera );


    // Window class members
    // --------------------
    
    SDL_Window * _SDL_window;
    SDL_GLContext _openGL_context;

    int _width;
    int _height;

};