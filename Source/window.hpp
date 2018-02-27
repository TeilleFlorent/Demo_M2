#include "camera.hpp"
#include "toolbox.hpp"


#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>

#define GLEW_STATIC
#include <GL/glew.h>

using namespace std;


//******************************************************************************
//**********  Class Window  ****************************************************
//******************************************************************************

class Scene;

class Window
{

  public:


    // Window functions
    // ----------------

    Window();

    void Quit();

    void Initialization();  

    void InitGL();

    SDL_Window * InitSDLWindow( int iWidth,
                                int iHeight,
                                SDL_GLContext * iOpenGLContext );

    void Resize();

    void ManageEvents( Camera * iCamera );

    void Draw();


    // Window class members
    // --------------------

    SDL_Window * _SDL_window;
    
    SDL_GLContext _openGL_context;
    
    Scene * _scene;

    Toolbox * _toolbox;

    int _width;

    int _height;

};

#endif  // WINDOW_H
