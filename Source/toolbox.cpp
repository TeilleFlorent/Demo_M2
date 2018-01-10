#include "toolbox.hpp"
#include "window.hpp"


//******************************************************************************
//**********  Class Toolbox  ***************************************************
//******************************************************************************

Toolbox::Toolbox( Window * iParentWindow )
{ 
  // Get pointer on the scene window
  _window = iParentWindow;

  _hdr_image_manager = new HDRManager();  

  _quad_VAO     = 0;
  _quad_VBO     = 0;

  _cube_VAO     = 0;
  _cube_VBO     = 0;

  _depth_map_res_seed     = 1024.0;
  _reflection_cubeMap_res = 512;
  _tex_VL_res_seed        = 2048.0;
}

void Toolbox::Quit()
{
  // Delete VAOs
  // -----------
  if( _quad_VAO )
    glDeleteVertexArrays( 1, &_quad_VAO );
  if( _cube_VAO )
    glDeleteVertexArrays( 1, &_cube_VAO );  
 

  // Delete VBOs
  // -----------
  if( _quad_VBO )
    glDeleteBuffers( 1, &_quad_VBO );
  if( _cube_VBO )
    glDeleteBuffers( 1, &_cube_VBO );
 
  if( _pingpong_FBO )
    glDeleteFramebuffers( 1, &_pingpong_FBO );
}

void Toolbox::PrintFPS()
{
  Uint32 t;
  static Uint32 t0 = 0, f = 0;
  f++;
  t = SDL_GetTicks();
  if( t - t0 > 1000 )
  {
    fprintf( stderr, "\nFPS -> %80.2f\n", ( 1000.0 * f / ( t - t0 ) ) );
    t0 = t;
    f  = 0;
  }
}

unsigned int Toolbox::LoadCubeMap( std::vector< const GLchar * > iPaths )
{
  unsigned int textureID;
  SDL_Surface * t = NULL;

  glGenTextures( 1, &textureID );
  glActiveTexture( GL_TEXTURE0 );

  glBindTexture( GL_TEXTURE_CUBE_MAP, textureID );

  for( unsigned int i = 0; i < iPaths.size(); i++ )
  {
    t = IMG_Load( iPaths[ i ] );
    glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels );
  }

  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
  glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );

  return textureID;
}

GLfloat * Toolbox::BuildSphere( int iLongitudes,
                                int iLatitudes )
{
  int i, j, k;
 
  GLfloat theta, phi, r[ 2 ], x[ 2 ], y[ 2 ], z[ 2 ], * data;
  GLfloat c2MPI_Long = 2.0 * _PI / iLongitudes;
  GLfloat cMPI_Lat = _PI / iLatitudes;
  data = new float[ ( 6 * 6 * iLongitudes * iLatitudes ) ];
 
  for( i = 0, k = 0; i < iLatitudes; i++ ) 
  {
    phi  = -_PI_2 + i * cMPI_Lat;
    y[ 0 ] = sin( phi );
    y[ 1 ] = sin( phi + cMPI_Lat );
    r[ 0 ] = cos( phi );
    r[ 1 ] = cos( phi  + cMPI_Lat );
    for( j = 0; j < iLongitudes; j++ )
    {
      theta = j * c2MPI_Long;
      x[ 0 ] = cos( theta );
      x[ 1 ] = cos( theta + c2MPI_Long );
      z[ 0 ] = sin( theta );
      z[ 1 ] = sin( theta + c2MPI_Long );

      // coordonné de vertex                                                        
      data[ k++ ] = r[ 0 ] * x[ 0 ]; data[ k++ ] = y[ 0 ];  data[ k++ ] = r[ 0 ] * z[ 0 ];      data[ k++ ] = 0.68;  data[ k++ ] = 0.0;  data[ k++ ] = 0.0;
      data[ k++ ] = r[ 1 ] * x[ 1 ]; data[ k++ ] = y[ 1 ];  data[ k++ ] = r[ 1 ] * z[ 1 ];      data[ k++ ] = 0.68;  data[ k++ ] = 0.0;  data[ k++ ] = 0.0;
      data[ k++ ] = r[ 0 ] * x[ 1 ]; data[ k++ ] = y[ 0 ];  data[ k++ ] = r[ 0 ] * z[ 1 ];      data[ k++ ] = 0.68;  data[ k++ ] = 0.0;  data[ k++ ] = 0.0;

      data[ k++ ] = r[ 0 ] * x[ 0 ]; data[ k++ ] = y[ 0 ];  data[ k++ ] = r[ 0 ] * z[ 0 ];      data[ k++ ] = 0.68;  data[ k++ ] = 0.0;  data[ k++ ] = 0.0;
      data[ k++ ] = r[ 1 ] * x[ 0 ]; data[ k++ ] = y[ 1 ];  data[ k++ ] = r[ 1 ] * z[ 0 ];      data[ k++ ] = 0.68;  data[ k++ ] = 0.0;  data[ k++ ] = 0.0;
      data[ k++ ] = r[ 1 ] * x[ 1 ]; data[ k++ ] = y[ 1 ];  data[ k++ ] = r[ 1 ] * z[ 1 ];      data[ k++ ] = 0.68;  data[ k++ ] = 0.0;  data[ k++ ] = 0.0;
    }
  }

  return data;
}

float Toolbox::RandFloatRange( float iMin,
                               float iMax )
{
  return ( ( iMax - iMin ) * ( ( float )rand() / RAND_MAX ) ) + iMin;
}

void Toolbox::PrintMatrix( glm::mat4 * iMatrix )
{
  const float * Source = ( const float* )glm::value_ptr( *iMatrix );
  
  printf( "\n" );
  for( int i = 0; i < 4; i++ )
  {
    for( int j = 0; j < 4; j++ )
    {
      printf( "%.3f ", Source[ ( 4 * j ) + i ] );
    }
    printf( "\n" );
  }
  printf( "\n" );
}

bool Toolbox::IsTextureRGBA( SDL_Surface * t )
{
  if( t->format->format == SDL_PIXELFORMAT_RGB332
   || t->format->format == SDL_PIXELFORMAT_RGB444
   || t->format->format == SDL_PIXELFORMAT_RGB555
   || t->format->format == SDL_PIXELFORMAT_RGB565
   || t->format->format == SDL_PIXELFORMAT_RGB24
   || t->format->format == SDL_PIXELFORMAT_RGB888
   //|| t->format->format == SDL_PIXELFORMAT_RGBX8888
   || t->format->format == SDL_PIXELFORMAT_RGB565
   || t->format->format == SDL_PIXELFORMAT_BGR555
   || t->format->format == SDL_PIXELFORMAT_BGR565
   || t->format->format == SDL_PIXELFORMAT_BGR24
   || t->format->format == SDL_PIXELFORMAT_BGR888 )
  {
    return false;
  }
  else
  {
    if( t->format->format == SDL_PIXELFORMAT_RGBA4444
    ||  t->format->format == SDL_PIXELFORMAT_RGBA5551
    ||  t->format->format == SDL_PIXELFORMAT_ARGB4444
    ||  t->format->format == SDL_PIXELFORMAT_ABGR4444
    ||  t->format->format == SDL_PIXELFORMAT_BGRA4444
    ||  t->format->format == SDL_PIXELFORMAT_ABGR1555
    ||  t->format->format == SDL_PIXELFORMAT_BGRA5551
    ||  t->format->format == SDL_PIXELFORMAT_ARGB8888
    ||  t->format->format == SDL_PIXELFORMAT_ABGR8888
    ||  t->format->format == SDL_PIXELFORMAT_BGRA8888
    //|| t->format->format == SDL_PIXELFORMAT_RGBX8888
    ||  t->format->format == SDL_PIXELFORMAT_RGBA8888 )
    {
      return true;
    }
    else
    { 
      return false;
    }
  }
}

void Toolbox::InitAudio()
{
  int mixFlags = MIX_INIT_MP3 | MIX_INIT_OGG;
  int res = Mix_Init(mixFlags);
  
  if( ( res & mixFlags ) != mixFlags )
  {
    fprintf( stderr, "Mix_Init: Erreur lors de l'initialisation de la bibliothèque SDL_Mixer\n" );
    fprintf( stderr, "Mix_Init: %s\n", Mix_GetError() );
  }

  if( Mix_OpenAudio(44100  /*22050*/ , /*AUDIO_S16LSB*/ MIX_DEFAULT_FORMAT, 2, 1024 ) < 0 )
  {
    printf("BUG init audio\n");  
    //exit(-4);
  }

  Mix_VolumeMusic( MIX_MAX_VOLUME / 3 );
  Mix_AllocateChannels( 10 );
}

void Toolbox::LoadAudio()
{

}

void Toolbox::RenderQuad()
{
  if( _quad_VAO == 0 )
  {
    GLfloat quad_vertices[] =
    {
      // Positions        // UV
      -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
       1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
       1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    // Setup plane VAO
    glGenVertexArrays( 1, &_quad_VAO );
    glGenBuffers( 1, &_quad_VBO );
    glBindVertexArray( _quad_VAO );
    glBindBuffer( GL_ARRAY_BUFFER, _quad_VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( quad_vertices ), &quad_vertices, GL_STATIC_DRAW );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ), ( GLvoid* )0 );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ), ( GLvoid* )( 3 * sizeof( GLfloat ) ) );
  }
  glBindVertexArray( _quad_VAO );
  glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
  glBindVertexArray( 0 );
}

void Toolbox::RenderCube()
{ 

  // Create cube VAO
  // ---------------
  if( _cube_VAO == 0 )
  {
    GLfloat vertices[] =
    {
      // Back face
      -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
       1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
       1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
       1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
      -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
      -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
      
      // Front face
      -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
       1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
       1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
       1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
      -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
      -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
      
      // Left face
      -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
      -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
      -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
      -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
      -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
      -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
      
      // Right face
      1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
      1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
      1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
      1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
      1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
      1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
      
      // Bottom face
      -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
       1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
       1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
       1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
      -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
      -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
      
      // Top face
      -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
       1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
       1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
       1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
      -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
      -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
    };

    glGenVertexArrays( 1, &_cube_VAO );
    glGenBuffers( 1, &_cube_VBO );
    glBindBuffer( GL_ARRAY_BUFFER, _cube_VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
    glBindVertexArray( _cube_VAO );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( GLfloat ), ( GLvoid* )0 );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( GLfloat ), ( GLvoid* )( 3 * sizeof( GLfloat ) ) );
    glEnableVertexAttribArray( 2 );
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof( GLfloat ), ( GLvoid* )( 6 * sizeof( GLfloat ) ) );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );
  }

  // Render Cube
  // -----------
  glBindVertexArray( _cube_VAO );
  glDrawArrays( GL_TRIANGLES, 0, 36 );
  glBindVertexArray( 0 );
}

void Toolbox::RenderObserver()
{
  // Set GL buffer 0
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // Draw observer
  glViewport( 0, 0, _window->_width, _window->_height );
  _window->_scene->_observer_shader.Use();
  glActiveTexture( GL_TEXTURE0 );
  //glBindTexture( GL_TEXTURE_2D, _window->_scene->_g_buffer_textures[ 5 ] );
  glBindTexture( GL_TEXTURE_2D, _pingpong_color_buffers[ 0 ] );
  //glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[ 1 ] /*final_tex_color_buffer[0]*/ /*pingpongColorbuffers[0]*/ /*tex_depth_ssr*/ );
  
  glUniform1f( glGetUniformLocation( _window->_scene->_observer_shader._program, "uCameraNear" ), _window->_scene->_camera->_near );
  glUniform1f( glGetUniformLocation( _window->_scene->_observer_shader._program, "uCameraFar" ), _window->_scene->_camera->_far );

  RenderQuad();
  glBindVertexArray( 0 );
  glUseProgram( 0 );
}

void Toolbox::SetFboTexture( unsigned int iTextureID,
                             GLenum       iFormat,
                             int          iWidth,
                             int          iHeight,
                             GLenum       iAttachment )
{
  glBindTexture( GL_TEXTURE_2D, iTextureID );
  glTexImage2D( GL_TEXTURE_2D, 0, iFormat, iWidth, iHeight, 0, GL_RGB, GL_FLOAT, NULL );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );  
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glFramebufferTexture2D( GL_FRAMEBUFFER, iAttachment, GL_TEXTURE_2D, iTextureID, 0 );
}

void Toolbox::SetFboMultiSampleTexture( unsigned int iTextureID,
                                        int          iSampleCount,
                                        GLenum       iFormat,
                                        int          iWidth,
                                        int          iHeight,
                                        GLenum       iAttachment )
{
  glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, iTextureID );
  glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, iSampleCount ,iFormat, iWidth, iHeight, GL_TRUE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );  
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glFramebufferTexture2D( GL_FRAMEBUFFER, iAttachment, GL_TEXTURE_2D_MULTISAMPLE, iTextureID, 0 );
}

void Toolbox::LinkRbo( unsigned int iRboID,
                       int iWidth,
                       int iHeight )
{
  glGenRenderbuffers( 1, &iRboID );
  glBindRenderbuffer( GL_RENDERBUFFER, iRboID );
  glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, iWidth, iHeight );
  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, iRboID );
  glBindRenderbuffer( GL_RENDERBUFFER, 0 );    
}

void Toolbox::LinkMultiSampleRbo( unsigned int iRboID,
                                  int          iSampleCount, 
                                  int          iWidth,
                                  int          iHeight )
{
  glGenRenderbuffers( 1, &iRboID );
  glBindRenderbuffer( GL_RENDERBUFFER, iRboID );
  glRenderbufferStorageMultisample( GL_RENDERBUFFER, iSampleCount, GL_DEPTH24_STENCIL8, iWidth, iHeight ); 
  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, iRboID );
  glBindRenderbuffer( GL_RENDERBUFFER, 0 );
}
