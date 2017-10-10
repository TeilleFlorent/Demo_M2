#include "window.hpp"
#include "scene.hpp"


//******************************************************************************
//**********  Class Window  ****************************************************
//******************************************************************************

Window::Window()
{
  _SDL_window     = NULL;
  _openGL_context = NULL;

  _width  = 800 * 1.5;
  _height = 600 * 1.5;

  Initialization();
  _toolbox = new Toolbox( this );
  _scene = new Scene( this );
}

void Window::Quit()
{
  // Delete window
  // -------------
  if( _openGL_context )
    SDL_GL_DeleteContext( _openGL_context );
  if( _SDL_window )
    SDL_DestroyWindow( _SDL_window );
}

void Window::Initialization()
{
  // Init SDL
  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
  {
    fprintf( stderr, "Erreur lors de l'initialisation de SDL :  %s", SDL_GetError() );
    exit( 1 );
  }
  atexit( SDL_Quit );

  // Init SDL window
  _SDL_window = InitSDLWindow( _width,
                               _height,
                               &_openGL_context );
  if( _SDL_window == NULL )
  {
    exit( 1 );
  }

  // Init OpenGL
  InitGL();

  // Init glew
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if( err != GLEW_OK )
  {
    exit( 1 );
  }
  if( !GLEW_VERSION_2_1 )
  {
    exit( 1 ); 
  }

  SDL_SetRelativeMouseMode( SDL_TRUE );
}

SDL_Window * Window::InitSDLWindow( int iWidth,
                                    int iHeight,
                                    SDL_GLContext * iOpenGLContext )
{
  SDL_Window * win = NULL;
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
  //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

  // Create SDL window
  win = SDL_CreateWindow( "OpenGL Demo",
                          SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED, 
                          iWidth,
                          iHeight,
                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/ );

  if( win == NULL )
  {
    return NULL;
  }

  // Create OpenGL context into the SDL window
  *iOpenGLContext = SDL_GL_CreateContext( win );

  if( iOpenGLContext == NULL )
  {
    SDL_DestroyWindow( win );
    return NULL;
  }

  fprintf( stderr, "\nVersion d'OpenGL : %s\n", glGetString( GL_VERSION ) );
  fprintf( stderr, "Version de shaders supportes : %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );  
  return win;
}

void Window::InitGL()
{
  glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LESS ); 

  Resize();

  //glEnable(GL_BLEND);
  //glBlendEquation(GL_FUNC_ADD);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 
  //glEnable(GL_MULTISAMPLE); // active anti aliasing 
  
  //glFrontFace(GL_CCW);
  //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
}

void Window::Resize()
{
  SDL_GetWindowSize( _SDL_window, &_width, &_height );

  //std::cout << "W = " << w << ", H = " << h << std::endl;

  SDL_WarpMouseInWindow( _SDL_window, _width / 2.0, _height / 2.0 );
}

void Window::ManageEvents( Camera * iCamera )
{
  SDL_Event event;


  // Key & window Event 
  // ------------------
  while( SDL_PollEvent( &event ) )
  {
    switch( event.type )
    {

      case SDL_KEYDOWN :
        switch( event.key.keysym.sym ) 
        {

          case SDLK_ESCAPE:
            exit( 0 );

          case 'z' :
            iCamera->_Z_state = 1;
            break;

          case 'q' :
            iCamera->_Q_state = 1;
            break;

          case 's' :
            iCamera->_S_state = 1;
            break;

          case 'd' :
            iCamera->_D_state = 1;
            break;

          case 'a' :
            for( int i = 0 ; i < _scene->_lights.size() ; i++ )
            {     
              _scene->_lights[ i ]._position.y += 0.5;
            }   
            break;

          case 'e' :
            for( int i = 0 ; i < _scene->_lights.size() ; i++ )
            {     
              _scene->_lights[ i ]._position.y -= 0.5;
            }     
            break;

          case 'r' :
            for( int i = 0 ; i < _scene->_tables.size() ; i++ )
            {     
              _scene->_tables[ i ]._angle += 0.02;
            }     
            break;
       
          case 'v' :
            _scene->_bloom = ( _scene->_bloom == true ) ? false : true;
            break;

          default:
            fprintf( stderr, "La touche %s a ete pressee\n", SDL_GetKeyName( event.key.keysym.sym ) );
            break;
        }
        break;

      case SDL_KEYUP :
        switch( event.key.keysym.sym ) 
        {
          case 'z' :
            iCamera->_Z_state = 0;
            break;

          case 'q' :
            iCamera->_Q_state = 0;
            break;

          case 's' :
            iCamera->_S_state = 0;
            break;

          case 'd' :
            iCamera->_D_state = 0;
            break;
        }
        break;

      case SDL_WINDOWEVENT :
        if( event.window.windowID == SDL_GetWindowID( _SDL_window ) ) 
        {
          switch( event.window.event )  
          {
            case SDL_WINDOWEVENT_RESIZED :
              SDL_GetWindowSize( _SDL_window, &_width, &_height );
              Resize();    
              _scene->SceneDataInitialization();
              break;
            case SDL_WINDOWEVENT_CLOSE :
              event.type = SDL_QUIT;
              SDL_PushEvent( &event );
              break;
          }
        }
        break;
      case SDL_QUIT:
        exit( 0 );
    }
  }
}