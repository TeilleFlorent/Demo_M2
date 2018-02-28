#include "toolbox.hpp"
#include "window.hpp"


//******************************************************************************
//**********  Class Toolbox  ***************************************************
//******************************************************************************

Toolbox::Toolbox()
{

}

Toolbox::Toolbox( Window * iParentWindow )
{ 
  // Get pointer on the scene window
  _window = iParentWindow;

  _hdr_image_manager = new HDRManager();  

  _quad_VAO     = 0;
  _quad_VBO     = 0;

  _cube_VAO     = 0;
  _cube_VBO     = 0;
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
  glBindTexture( GL_TEXTURE_2D, _temp_tex_color_buffer[ 0 ] );
  //glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[ 1 ] /*final_tex_color_buffer[0]*/ /*pingpongColorbuffers[0]*/ /*tex_depth_ssr*/ );
  //glBindTexture( GL_TEXTURE_2D, _window->_scene->_pre_brdf_texture );
  
  glUniform1f( glGetUniformLocation( _window->_scene->_observer_shader._program, "uCameraNear" ), _window->_scene->_camera->_near );
  glUniform1f( glGetUniformLocation( _window->_scene->_observer_shader._program, "uCameraFar" ), _window->_scene->_camera->_far );

  RenderQuad();
  glBindVertexArray( 0 );
  glUseProgram( 0 );
}

unsigned int Toolbox::CreateTextureFromData( SDL_Surface * iImage,
                                             int           iInternalFormat,
                                             int           iFormat,
                                             bool          iMipmap,
                                             bool          iAnisotropy,
                                             float         iAnisotropyValue )
{
  unsigned int result_id;

  glGenTextures( 1, &result_id );
  glBindTexture( GL_TEXTURE_2D, result_id );

  glTexImage2D( GL_TEXTURE_2D, 0, iInternalFormat, iImage->w, iImage->h, 0, iFormat, GL_UNSIGNED_BYTE, iImage->pixels );

  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ( iMipmap == true ) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  
  if( iAnisotropy )
  {
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, iAnisotropyValue ); // anisotropie
  }

  if( iMipmap )
  {
    glGenerateMipmap( GL_TEXTURE_2D );
  }

  glBindTexture( GL_TEXTURE_2D, 0 );

  return result_id;
}

unsigned int Toolbox::CreateEmptyTexture( int   iWidth,
                                          int   iHeight,
                                          int   iInternalFormat,
                                          int   iFormat,
                                          bool  iMipmap,
                                          bool  iAnisotropy,
                                          float iAnisotropyValue )
{
  unsigned int result_id;

  glGenTextures( 1, &result_id );
  glBindTexture( GL_TEXTURE_2D, result_id );

  glTexImage2D( GL_TEXTURE_2D, 0, iInternalFormat, iWidth, iHeight, 0, iFormat, GL_FLOAT, 0 );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ( iMipmap == true ) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  if( iAnisotropy )
  {
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, iAnisotropyValue ); // anisotropie
  }

  if( iMipmap )
  {
    glGenerateMipmap( GL_TEXTURE_2D );
  }

  glBindTexture( GL_TEXTURE_2D, 0 );

  return result_id;
}

unsigned int Toolbox::CreateCubeMapTexture( int  iResolution,
                                            bool iMipmap )
{
  unsigned int result_id;

  glGenTextures( 1, &result_id );
  glBindTexture( GL_TEXTURE_CUBE_MAP, result_id );

  for( unsigned int i = 0; i < 6; ++i )
  {
    glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
    0, 
    GL_RGB16F, 
    iResolution, 
    iResolution, 
    0, 
    GL_RGB, 
    GL_FLOAT, 
    nullptr );
  }

  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, ( iMipmap == true ) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  if( iMipmap )
  {
    glGenerateMipmap( GL_TEXTURE_CUBE_MAP ); // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory 
  }

  return result_id;
}

void Toolbox::SetFboTexture( unsigned int iTextureID,
                             int          iFormat,
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
                                        int          iFormat,
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

glm::mat4 Toolbox::AssimpMatrixToGlmMatrix( const aiMatrix4x4 * iAssimpMatrix )
{
  glm::mat4 result_matrix;

  result_matrix[ 0 ][ 0 ] = ( GLfloat )iAssimpMatrix->a1; 
  result_matrix[ 1 ][ 0 ] = ( GLfloat )iAssimpMatrix->a2; 
  result_matrix[ 2 ][ 0 ] = ( GLfloat )iAssimpMatrix->a3; 
  result_matrix[ 3 ][ 0 ] = ( GLfloat )iAssimpMatrix->a4; 

  result_matrix[ 0 ][ 1 ] = ( GLfloat )iAssimpMatrix->b1;  
  result_matrix[ 1 ][ 1 ] = ( GLfloat )iAssimpMatrix->b2;  
  result_matrix[ 2 ][ 1 ] = ( GLfloat )iAssimpMatrix->b3;  
  result_matrix[ 3 ][ 1 ] = ( GLfloat )iAssimpMatrix->b4;  

  result_matrix[ 0 ][ 2 ] = ( GLfloat )iAssimpMatrix->c1; 
  result_matrix[ 1 ][ 2 ] = ( GLfloat )iAssimpMatrix->c2; 
  result_matrix[ 2 ][ 2 ] = ( GLfloat )iAssimpMatrix->c3; 
  result_matrix[ 3 ][ 2 ] = ( GLfloat )iAssimpMatrix->c4; 

  result_matrix[ 0 ][ 3 ] = ( GLfloat )iAssimpMatrix->d1;
  result_matrix[ 1 ][ 3 ] = ( GLfloat )iAssimpMatrix->d2;
  result_matrix[ 2 ][ 3 ] = ( GLfloat )iAssimpMatrix->d3;
  result_matrix[ 3 ][ 3 ] = ( GLfloat )iAssimpMatrix->d4;

  return result_matrix;
}

void Toolbox::CreatePlaneVAO( unsigned int *                iVAO,
                              unsigned int *                iVBO,
                              unsigned int *                iIBO,
                              std::vector< unsigned int > * iIndices,
                              unsigned int                  iSideVerticeCount,
                              float                         iUvScale )
{
  unsigned int width_count  = iSideVerticeCount;
  unsigned int height_count = iSideVerticeCount;
  float width_size          = 1.0;
  float height_size         = 1.0;
  std::vector< float > plane_vertices;

  // Create plane vertices
  for( unsigned int width_it = 0; width_it < width_count; width_it ++ )
  {
    for( unsigned int height_it = 0; height_it < height_count; height_it ++ )
    { 
      // position
      float pos_x = ( ( float )width_it / ( float )( width_count - 1 ) ) * width_size;
      float pos_y = 0.0;         
      float pos_z = ( ( float )height_it / ( float )( height_count - 1 ) ) * height_size;
      plane_vertices.push_back( pos_x );
      plane_vertices.push_back( pos_y );
      plane_vertices.push_back( pos_z );

      // normal
      plane_vertices.push_back( 0.0f );
      plane_vertices.push_back( -1.0f );
      plane_vertices.push_back( 0.0f );    

      // uv    
      float u = ( float )width_it / ( float )( width_count - 1 );
      float v = ( float )height_it / ( float )( height_count - 1 );
      plane_vertices.push_back( u * iUvScale );
      plane_vertices.push_back( v * iUvScale );

      // tangent
      plane_vertices.push_back( -1.0 );
      plane_vertices.push_back( 0.0 );
      plane_vertices.push_back( 0.0 );

      // bitangent
      plane_vertices.push_back( 0.0 );
      plane_vertices.push_back( 0.0 );
      plane_vertices.push_back( -1.0 );
    } 
  }

  // Create plane indices
  for( unsigned int width_it = 0; width_it < width_count - 1; width_it ++ )
  {
    for( unsigned int height_it = 0; height_it < height_count - 1; height_it ++ )
    { 
      // Get base vertex index
      unsigned int vertex_index = height_it * width_count + width_it;
      
      // Gen two triangles indices
      iIndices->push_back( vertex_index );       
      iIndices->push_back( vertex_index + width_count );
      iIndices->push_back( vertex_index + width_count + 1 );
      iIndices->push_back( vertex_index );
      iIndices->push_back( vertex_index + width_count + 1 );
      iIndices->push_back( vertex_index + 1 );     
    }   
  }

  // Setup plane VAO & IBO
  glGenVertexArrays( 1, iVAO );
  glBindVertexArray( *iVAO );

  glGenBuffers( 1, iVBO );
  glBindBuffer( GL_ARRAY_BUFFER, *iVBO );
  glBufferData( GL_ARRAY_BUFFER, plane_vertices.size() * sizeof( GLfloat ), plane_vertices.data(), GL_STATIC_DRAW );

  glGenBuffers( 1, iIBO );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, *iIBO );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, iIndices->size() * sizeof( unsigned int ), iIndices->data(), GL_STATIC_DRAW );

  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof( GLfloat ), ( GLvoid* )0 );
  glEnableVertexAttribArray( 1 );
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof( GLfloat ), ( GLvoid* )( 3 * sizeof( GLfloat ) ) );
  glEnableVertexAttribArray( 2 );
  glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof( GLfloat ), ( GLvoid* )( 6 * sizeof( GLfloat ) ) );
  glEnableVertexAttribArray( 3 );
  glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof( GLfloat ), ( GLvoid* )( 8 * sizeof( GLfloat ) ) );
  glEnableVertexAttribArray( 4 );
  glVertexAttribPointer( 4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof( GLfloat ), ( GLvoid* )( 11 * sizeof( GLfloat ) ) );

  glBindVertexArray( 0 );
}

unsigned int Toolbox::GenIrradianceCubeMap( unsigned int iEnvCubeMap,
                                            unsigned int iResCubeMap,
                                            Shader       iIrradianceShader,
                                            float        iIrradianceSampleDelta )
{
  unsigned int capture_FBO;
  unsigned int capture_RBO;


  // Gen irradiance cube map textures
  // --------------------------------
  unsigned int irradiance_cubemap = _window->_toolbox->CreateCubeMapTexture( iResCubeMap,
                                                                             false );


  // Gen FBO and RBO to render hdr tex into cube map tex
  // ---------------------------------------------------
  glGenFramebuffers( 1, &capture_FBO );
  glGenRenderbuffers( 1, &capture_RBO );
  glBindFramebuffer( GL_FRAMEBUFFER, capture_FBO );
  glBindRenderbuffer( GL_RENDERBUFFER, capture_RBO );
  glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, iResCubeMap, iResCubeMap );
  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_RBO );


  // Create 6 matrix to each cube map face
  // -------------------------------------
  glm::mat4 capture_projection_matrix = glm::perspective( glm::radians( 90.0f ), 1.0f, 0.1f, 10.0f );
  glm::mat4 capture_view_matrices[] =
  {
    glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  1.0f,  0.0f,  0.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) ),
    glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( -1.0f,  0.0f,  0.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) ),
    glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  0.0f,  1.0f,  0.0f ), glm::vec3( 0.0f,  0.0f,  1.0f ) ),
    glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  0.0f, -1.0f,  0.0f ), glm::vec3( 0.0f,  0.0f, -1.0f ) ),
    glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  0.0f,  0.0f,  1.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) ),
    glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  0.0f,  0.0f, -1.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) )
  };


  // Compute diffuse irradiance cube map  
  // -----------------------------------
  iIrradianceShader.Use();
  glUniformMatrix4fv( glGetUniformLocation( iIrradianceShader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_projection_matrix ) );
  glUniform1f( glGetUniformLocation( iIrradianceShader._program, "uSampleDelta" ), iIrradianceSampleDelta );
  glUniform1i( glGetUniformLocation( iIrradianceShader._program, "uEnvironmentMap" ), 0 );

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, iEnvCubeMap );
  
  glViewport( 0, 0, iResCubeMap, iResCubeMap ); // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer( GL_FRAMEBUFFER, capture_FBO );
  for( unsigned int i = 0; i < 6; ++i )
  {
    glUniformMatrix4fv( glGetUniformLocation( iIrradianceShader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_view_matrices[ i ] ) );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_cubemap, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    RenderCube();
  }
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );


  // return computed irradiance cubemap
  // ----------------------------------
  return irradiance_cubemap;
}

unsigned int Toolbox::GenPreFilterCubeMap( unsigned int iEnvCubeMap,
                                           unsigned int iResCubeMap,
                                           Shader       iPrefilterShader,
                                           unsigned int iPrefilterSampleCount,
                                           unsigned int iPrefilterMaxMipLevel )
{
  unsigned int capture_FBO;
  unsigned int capture_RBO;


  // Create pre filter cube map textures
  // -----------------------------------
  unsigned int pre_filter_cubemap = CreateCubeMapTexture( iResCubeMap,
                                                          true );


  // Gen FBO and RBO to render hdr tex into cube map tex
  // ---------------------------------------------------
  glGenFramebuffers( 1, &capture_FBO );
  glGenRenderbuffers( 1, &capture_RBO );
  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_RBO );


  // Create 6 matrix to each cube map face
  // -------------------------------------
  glm::mat4 capture_projection_matrix = glm::perspective( glm::radians( 90.0f ), 1.0f, 0.1f, 10.0f );
  glm::mat4 capture_view_matrices[] =
  {
    glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  1.0f,  0.0f,  0.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) ),
    glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( -1.0f,  0.0f,  0.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) ),
    glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  0.0f,  1.0f,  0.0f ), glm::vec3( 0.0f,  0.0f,  1.0f ) ),
    glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  0.0f, -1.0f,  0.0f ), glm::vec3( 0.0f,  0.0f, -1.0f ) ),
    glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  0.0f,  0.0f,  1.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) ),
    glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  0.0f,  0.0f, -1.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) )
  };


  // Compute specular pre filter cube map
  // ------------------------------------
  iPrefilterShader.Use();
  glUniform1i( glGetUniformLocation( iPrefilterShader._program, "uEnvironmentMap" ), 0 );
  glUniformMatrix4fv( glGetUniformLocation( iPrefilterShader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_projection_matrix ) );
  glUniform1f( glGetUniformLocation( iPrefilterShader._program, "uCubeMapRes" ), iResCubeMap );
  glUniform1ui( glGetUniformLocation( iPrefilterShader._program, "uSampleCount" ), iPrefilterSampleCount );

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, iEnvCubeMap );

  glBindFramebuffer( GL_FRAMEBUFFER, capture_FBO );

  for( unsigned int mip = 0; mip < iPrefilterMaxMipLevel; mip++ )
  {
    // Reise framebuffer according to mip-level size.
    unsigned int mip_width  = iResCubeMap * std::pow( 0.5, mip );
    unsigned int mip_height = mip_width;
    glBindRenderbuffer( GL_RENDERBUFFER, capture_RBO );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_width, mip_height );
    glViewport( 0, 0, mip_width, mip_height );

    // Set roughness level for wich we need to render
    float roughness = ( float )mip / ( float )( iPrefilterMaxMipLevel - 1 );
    glUniform1f( glGetUniformLocation( iPrefilterShader._program, "uRoughness" ), roughness );

    // Compute pre filter cube map for a given mip level and roughness level
    for( unsigned int i = 0; i < 6; i++ )
    {
      glUniformMatrix4fv( glGetUniformLocation( iPrefilterShader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_view_matrices[ i ] ) );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, pre_filter_cubemap, mip );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      RenderCube();
    }
  }
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );


  return pre_filter_cubemap;
}

std::vector< unsigned int > Toolbox::LoadMaterialTextures( std::string iMaterialName,
                                                           float       iAnisotropy,
                                                           bool        iEmissive )
{
  SDL_Surface * sdl_image_data = NULL;
  std::vector< unsigned int > material;
  
  std::string path( "" );
  path += "../Textures/materials/" + iMaterialName + "/";

  // albedo
  if( ( sdl_image_data = IMG_Load( std::string( path + "albedo.png" ).data() ) ) != NULL )
  {
    material.push_back( CreateTextureFromData( sdl_image_data,
                                               GL_RGB,
                                               GL_RGB,
                                               true,
                                               true,
                                               iAnisotropy ) );
    SDL_FreeSurface( sdl_image_data );

  }
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
  }

  // normal
  if( ( sdl_image_data = IMG_Load( std::string( path + "normal.png" ).data() ) ) != NULL )
  {
    material.push_back( CreateTextureFromData( sdl_image_data,
                                               GL_RGB,
                                               GL_RGB,
                                               true,
                                               true,
                                               iAnisotropy ) );
    SDL_FreeSurface( sdl_image_data );
  }
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
  }

  // height
  if( ( sdl_image_data = IMG_Load( std::string( path + "height.png" ).data() ) ) != NULL )
  {
    material.push_back( CreateTextureFromData( sdl_image_data,
                                               GL_R8,
                                               GL_RED,
                                               true,
                                               true,
                                               iAnisotropy ) );
    SDL_FreeSurface( sdl_image_data );
  }
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
  }

  // AO
  if( ( sdl_image_data = IMG_Load( std::string( path + "AO.png" ).data() ) ) != NULL )
  {
    material.push_back( CreateTextureFromData( sdl_image_data,
                                               GL_R8,
                                               GL_RED,
                                               true,
                                               true,
                                               iAnisotropy ) );
    SDL_FreeSurface( sdl_image_data );
  }
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
  }

  // Roughness
  if( ( sdl_image_data = IMG_Load( std::string( path + "roughness.png" ).data() ) ) != NULL )
  {
    material.push_back( CreateTextureFromData( sdl_image_data,
                                               GL_R8,
                                               GL_RED,
                                               true,
                                               true,
                                               iAnisotropy ) );
    SDL_FreeSurface( sdl_image_data );
  }
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
  }

  // Metalness
  if( ( sdl_image_data = IMG_Load( std::string( path + "metalness.png" ).data() ) ) != NULL )
  {
    material.push_back( CreateTextureFromData( sdl_image_data,
                                               GL_R8,
                                               GL_RED,
                                               true,
                                               true,
                                               iAnisotropy ) );
    SDL_FreeSurface( sdl_image_data );
  }
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
  }

  // Emissive
  if( iEmissive )
  {
    if( ( sdl_image_data = IMG_Load( std::string( path + "emissive.png" ).data() ) ) != NULL )
    {
      material.push_back( CreateTextureFromData( sdl_image_data,
                                                 GL_RGB,
                                                 GL_RGB,
                                                 true,
                                                 true,
                                                 iAnisotropy ) );
      SDL_FreeSurface( sdl_image_data );
    }
    else
    {
      fprintf( stderr, "Erreur lors du chargement de la texture\n" );
    }
  }  

  return material;
}

unsigned int Toolbox::GenEnvironmentCubemap( glm::vec3    iPosition,
                                             bool         iNeedAllWalls,
                                             unsigned int iWallID )
{
  unsigned int cubemap_id;
  unsigned int capture_FBO;
  unsigned int capture_RBO;


  // Gen FBO and RBO to render environement into cube map tex
  // --------------------------------------------------------
  glGenFramebuffers( 1, &capture_FBO );
  glGenRenderbuffers( 1, &capture_RBO );
  glBindFramebuffer( GL_FRAMEBUFFER, capture_FBO );
  glBindRenderbuffer( GL_RENDERBUFFER, capture_RBO );
  glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _window->_scene->_res_env_cubemap, _window->_scene->_res_env_cubemap );
  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_RBO );


  // Gen the output cubemap textures
  // -------------------------------
  glGenTextures( 1, &cubemap_id );
  glBindTexture( GL_TEXTURE_CUBE_MAP, cubemap_id );
  for( unsigned int i = 0; i < 6; ++i )
  {
    glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
    0, 
    GL_RGB16F, 
    _window->_scene->_res_env_cubemap, 
    _window->_scene->_res_env_cubemap, 
    0, 
    GL_RGB, 
    GL_FLOAT, 
    nullptr );
  }
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ); // enable pre-filter mipmap sampling (combatting visible dots artifact)
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );


  // Create 6 matrix to each cube map face
  // -------------------------------------
  glm::mat4 capture_projection_matrix = glm::perspective( glm::radians( 90.0f ), 1.0f, _window->_scene->_near, _window->_scene->_far );
  glm::mat4 capture_view_matrices[] =
  {
    glm::lookAt( iPosition, iPosition + glm::vec3(  1.0f,  0.0f,  0.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) ),
    glm::lookAt( iPosition, iPosition + glm::vec3( -1.0f,  0.0f,  0.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) ),
    glm::lookAt( iPosition, iPosition + glm::vec3(  0.0f,  1.0f,  0.0f ), glm::vec3( 0.0f,  0.0f,  1.0f ) ),
    glm::lookAt( iPosition, iPosition + glm::vec3(  0.0f, -1.0f,  0.0f ), glm::vec3( 0.0f,  0.0f, -1.0f ) ),
    glm::lookAt( iPosition, iPosition + glm::vec3(  0.0f,  0.0f,  1.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) ),
    glm::lookAt( iPosition, iPosition + glm::vec3(  0.0f,  0.0f, -1.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) )
  };


  // Bind capture FBO to render into cubemap texture
  // -----------------------------------------------
  glViewport( 0, 0, _window->_scene->_res_env_cubemap, _window->_scene->_res_env_cubemap );
  glBindFramebuffer( GL_FRAMEBUFFER, capture_FBO );

  // Use the correct shader 
  _window->_scene->_forward_pbr_shader.Use();

  glm::mat4 model_matrix;
  for( unsigned int i = 0; i < 6; ++i )
  { 
    // Bind correct cubemap face to render
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap_id, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


    // Draw grounds type 1
    // -------------------
    for( int ground_it = 0; ground_it < _window->_scene->_grounds_type1.size(); ground_it ++ )
    {
      model_matrix = _window->_scene->_grounds_type1[ ground_it ]._model_matrix; 
     
      // Textures binding
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_grounds_type1[ ground_it ]._material_id ][ 0 ] );  
      glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_grounds_type1[ ground_it ]._material_id ][ 1 ] ); 
      glActiveTexture( GL_TEXTURE2 );
      glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_grounds_type1[ ground_it ]._material_id ][ 2 ] ); 
      glActiveTexture( GL_TEXTURE3 );
      glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_grounds_type1[ ground_it ]._material_id ][ 3 ] ); 
      glActiveTexture( GL_TEXTURE4 );
      glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_grounds_type1[ ground_it ]._material_id ][ 4 ] ); 
      glActiveTexture( GL_TEXTURE5 );
      glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_grounds_type1[ ground_it ]._material_id ][ 5 ] ); 

      // Matrices uniforms
      glUniformMatrix4fv( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_projection_matrix ) );
      glUniformMatrix4fv( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_view_matrices[ i ] ) );
      glUniformMatrix4fv( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
      glUniform3fv( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uViewPos" ), 1, &iPosition[ 0 ] );

      // Point lights uniforms
      glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uLightCount" ), _window->_scene->_lights.size() );
      for( int i = 0; i < _window->_scene->_lights.size(); i++ )
      {
        string temp = to_string( i );
        glUniform3fv( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, ( "uLightPos[" + temp + "]" ).c_str() ),1, &_window->_scene->_lights[ i ]._position[ 0 ] );
        glUniform3fv( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, ( "uLightColor[" + temp + "]" ).c_str() ),1, &_window->_scene->_lights[ i ]._color[ 0 ] );
        glUniform1f(  glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, ( "uLightIntensity[" + temp + "]" ).c_str() ), _window->_scene->_lights[ i ]._intensity );
      }

      // Bloom uniforms
      glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uBloom" ), false );

      // IBL uniforms
      glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uIBL" ), false );

      // Omnidirectional shadow mapping uniforms
      glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uReceivShadow" ), false );

      // Opacity uniforms 
      glUniform1f( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uAlpha" ), _window->_scene->_grounds_type1[ ground_it ]._alpha );
      glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uOpacityMap" ), _window->_scene->_grounds_type1[ ground_it ]._opacity_map );
      glUniform1f( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
      
      // Displacement mapping uniforms
      glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uNormalMap" ), _window->_scene->_grounds_type1[ ground_it ]._normal_map );

      // Emissive uniforms
      glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uEmissive" ), _window->_scene->_grounds_type1[ ground_it ]._emissive );
      if( _window->_scene->_grounds_type1[ ground_it ]._emissive )
      {
        glActiveTexture( GL_TEXTURE11 );
        glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_grounds_type1[ ground_it ]._material_id ][ 6 ] );
        glUniform1f( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uEmissiveFactor" ), _window->_scene->_grounds_type1[ ground_it ]._emissive_factor );
      }

      glUniform1f( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uID" ), _window->_scene->_grounds_type1[ ground_it ]._id );  

      // Bind correct VAO
      ( _window->_scene->_grounds_type1[ ground_it ]._id == 18 ) ? glBindVertexArray( _window->_scene->_ground2_VAO ) : glBindVertexArray( _window->_scene->_ground1_VAO );
      
      glDrawElements( GL_TRIANGLES, _window->_scene->_ground1_indices.size(), GL_UNSIGNED_INT, 0 );
      
      glBindVertexArray( 0 );
    }


    // Draw walls type 1
    // -----------------
    if( iNeedAllWalls )
    {
      for( unsigned int wall_it = 0; wall_it < _window->_scene->_walls_type1.size(); wall_it++ )
      {
        model_matrix = _window->_scene->_walls_type1[ wall_it ]._model_matrix;

        // Textures binding
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_walls_type1[ wall_it ]._material_id ][ 0 ] );  
        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_walls_type1[ wall_it ]._material_id ][ 1 ] ); 
        glActiveTexture( GL_TEXTURE2 );
        glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_walls_type1[ wall_it ]._material_id ][ 2 ] ); 
        glActiveTexture( GL_TEXTURE3 );
        glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_walls_type1[ wall_it ]._material_id ][ 3 ] ); 
        glActiveTexture( GL_TEXTURE4 );
        glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_walls_type1[ wall_it ]._material_id ][ 4 ] ); 
        glActiveTexture( GL_TEXTURE5 );
        glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_walls_type1[ wall_it ]._material_id ][ 5 ] ); 

        // Matrices uniforms
        glUniformMatrix4fv( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_projection_matrix ) );
        glUniformMatrix4fv( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_view_matrices[ i ] ) );
        glUniformMatrix4fv( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
        glUniform3fv( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uViewPos" ), 1, &iPosition[ 0 ] );

        // Point lights uniforms
        glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uLightCount" ), _window->_scene->_lights.size() );
        for( int i = 0; i < _window->_scene->_lights.size(); i++ )
        {
          string temp = to_string( i );
          glUniform3fv( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, ( "uLightPos[" + temp + "]" ).c_str() ),1, &_window->_scene->_lights[ i ]._position[ 0 ] );
          glUniform3fv( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, ( "uLightColor[" + temp + "]" ).c_str() ),1, &_window->_scene->_lights[ i ]._color[ 0 ] );
          glUniform1f(  glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, ( "uLightIntensity[" + temp + "]" ).c_str() ), _window->_scene->_lights[ i ]._intensity );
        }

        // Bloom uniforms
        glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uBloom" ), false );

        // IBL uniforms
        glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uIBL" ), false );

        // Omnidirectional shadow mapping uniforms
        glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uReceivShadow" ), false );

        // Opacity uniforms
        glUniform1f( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uAlpha" ), _window->_scene->_walls_type1[ wall_it ]._alpha );
        glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uOpacityMap" ), _window->_scene->_walls_type1[ wall_it ]._opacity_map );
        glUniform1f( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
        
        // Displacement mapping uniforms
        glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uNormalMap" ), _window->_scene->_walls_type1[ wall_it ]._normal_map );

        // Emissive uniforms
        glUniform1i( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uEmissive" ), _window->_scene->_walls_type1[ wall_it ]._emissive );
        if( _window->_scene->_walls_type1[ wall_it ]._emissive )
        {
          glActiveTexture( GL_TEXTURE11 );
          glBindTexture( GL_TEXTURE_2D, _window->_scene->_loaded_materials[ _window->_scene->_walls_type1[ wall_it ]._material_id ][ 6 ] );
          glUniform1f( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uEmissiveFactor" ), _window->_scene->_walls_type1[ wall_it ]._emissive_factor );
        }
        
        glUniform1f( glGetUniformLocation( _window->_scene->_forward_pbr_shader._program, "uID" ), _window->_scene->_walls_type1[ wall_it ]._id );  

        // Bind correct VAO
        ( _window->_scene->_walls_type1[ wall_it ]._id == 4 ) ? glBindVertexArray( _window->_scene->_wall2_VAO ) : glBindVertexArray( _window->_scene->_wall1_VAO );

        glDrawElements( GL_TRIANGLES, _window->_scene->_wall1_indices.size(), GL_UNSIGNED_INT, 0 );

        glBindVertexArray( 0 );
      }
    }
  }

  // generate mipmaps from first mip face ( combatting visible dots artifact )
  glBindTexture( GL_TEXTURE_CUBE_MAP, cubemap_id );
  glGenerateMipmap( GL_TEXTURE_CUBE_MAP ); 
  
  return cubemap_id;
}

