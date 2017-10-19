#include "main.hpp"


//******************************************************************************
//**********  Pipeline functions  **********************************************
//******************************************************************************

int main()
{
  // Init random seed
  srand( time( NULL ) );

  // Run the program loop
  Loop();

  return 0;
}

void Loop() 
{
  SDL_GL_SetSwapInterval( 1 );

  // Create and init window
  Window * window = new Window();

  // Get pointer on the scene
  Scene * scene = window->_scene;

  // Get pointer on the scene's camera
  Camera * camera = scene->_camera;

  // Get pointer on the scene's clock
  Clock * clock = scene->_clock;

  for(;;)
  {
    window->ManageEvents( camera );

    clock->TimeUpdate();
    
    camera->CameraUpdate( clock->_delta_time );

    window->Draw();

    window->_toolbox->PrintFPS();

    SDL_GL_SwapWindow( window->_SDL_window );
  }
}
