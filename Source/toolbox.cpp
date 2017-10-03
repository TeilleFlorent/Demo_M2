#include "toolbox.hpp"


//******************************************************************************
//**********  Class Toolbox  ***************************************************
//******************************************************************************

Toolbox::Toolbox()
{
  
}

void Toolbox::PrintFPS()
{
  Uint32 t;
  static Uint32 t0 = 0, f = 0;
  f++;
  t = SDL_GetTicks();
  if( t - t0 > 1000 )
  {
    fprintf( stderr, "Fps -> %80.2f\n", ( 1000.0 * f / ( t - t0 ) ) );
    t0 = t;
    f  = 0;
  }
}

GLuint Toolbox::LoadCubeMap( std::vector< const GLchar * > iPaths )
{
  GLuint textureID;
  SDL_Surface * t = NULL;

  glGenTextures( 1, &textureID );
  glActiveTexture( GL_TEXTURE0 );

  glBindTexture( GL_TEXTURE_CUBE_MAP, textureID );

  for( GLuint i = 0; i < iPaths.size(); i++ )
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
