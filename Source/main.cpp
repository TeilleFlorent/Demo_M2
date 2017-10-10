#include "main.hpp"


//******************************************************************************
//**********  Pipeline functions  **********************************************
//******************************************************************************

int main()
{
  // Init srand seed
  srand( time( NULL ) );

  // Create and init window
  _window = new Window();

  // Init scene data
  _window->_scene->SceneDataInitialization();

  atexit( Quit );

  // Run the program loop
  Loop( _window->_SDL_window );

  return 0;
}

void Quit()
{ 
  std::cout << std::endl << "Program Quit" << std::endl;
  _window->Quit();
  _window->_scene->Quit();
}

void Loop( SDL_Window * iWindow ) 
{
  SDL_GL_SetSwapInterval( 1 );
  
  // Get pointer on the scene
  Scene * scene = _window->_scene;

  for(;;)
  {
    _window->ManageEvents( scene->_camera );

    scene->_clock->TimeUpdate();
    
    scene->_camera->CameraUpdate( scene->_clock->_delta_time );

    Draw();

    _window->_toolbox->PrintFPS();

    SDL_GL_SwapWindow( iWindow );
  }
}

void Draw() 
{ 
  // Get pointer on the scene
  Scene * scene = _window->_scene;


  // Scene rendering
  // --------------- 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

  // Render observer
  _window->_toolbox->RenderObserver();
 
  // Render scene
  glViewport( 0, 0, _window->_width, _window->_height );
  scene->RenderScene( true );

  // Blur calculation on bright texture
  glViewport( 0, 0, _window->_width * scene->_bloom_downsample, _window->_height * scene->_bloom_downsample );
  scene->BlurProcess();

  // Bloom blending calculation => final render
  glViewport( 0, 0, _window->_width, _window->_height );
  scene->BloomProcess();

  glUseProgram( 0 );
}
