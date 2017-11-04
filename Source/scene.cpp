#include "scene.hpp"
#include "window.hpp"


//******************************************************************************
//**********  Class Scene  *****************************************************
//******************************************************************************

Scene::Scene( Window * iParentWindow )
{ 
  //_pipeline_type = FORWARD_RENDERING;
  _pipeline_type = DEFERRED_RENDERING;


  // Scene effects settings
  // ----------------------
  
  // Init bloom param
  _exposure         = 1.0;
  _bloom            = false;
  _bloom_downsample = 0.5;

  // Init multi sample param
  _multi_sample    = false;
  _nb_multi_sample = 2;

  // Init IBL param
  _res_IBL_cubeMap         = 512;
  _res_irradiance_cubeMap  = 32;
  _irradiance_sample_delta = 0.025;
  _current_env             = 2;


  // Scene data initialization
  // -------------------------
  
  _lampVAO   = 0;
  _lampVBO   = 0;

  _groundVAO = 0;
  _groundVBO = 0;

  _tex_albedo_ground    = 0;
  _tex_normal_ground    = 0;
  _tex_height_ground    = 0;
  _tex_AO_ground        = 0;
  _tex_roughness_ground = 0;
  _tex_metalness_ground = 0;


  // Get pointer on the scene window
  _window = iParentWindow;
  
  // Create and init scene clock
  _clock = new Clock();

   // Create and init scene camera
  _camera = new Camera();

  // Create and init all shaders
  ShadersInitialization();

  // Create all scene's objects
  ObjectsInitialization();

  // Load all scene models
  ModelsLoading();

  // Create lights
  LightsInitialization();  

  // Init scene data 
  SceneDataInitialization();

  // Init all IBL cubemap
  IBLCubeMapsInitialization();

  // Init deferred rendering g-buffer
  if( _pipeline_type == DEFERRED_RENDERING )
  {
    DeferredBuffersInitialization();
  }
}

void Scene::Quit()
{

  // Delete textures
  // ---------------
  if( _window->_toolbox->_pingpong_color_buffers[ 0 ] )
    glDeleteTextures( 1, &_window->_toolbox->_pingpong_color_buffers[ 0 ] );
  if( _window->_toolbox->_pingpong_color_buffers[ 1 ] )
    glDeleteTextures( 1, &_window->_toolbox->_pingpong_color_buffers[ 1 ] );
  for( int i = 0; i < _hdr_textures.size(); i++ )
  {
    if( _hdr_textures[ i ] )
      glDeleteTextures( 1, &_hdr_textures[ i ] );
    if( _env_cubemaps[ i ] )
      glDeleteTextures( 1, &_env_cubemaps[ i ] );
    if( _irradiance_maps[ i ] )
      glDeleteTextures( 1, &_irradiance_maps[ i ] );
  }
  if( _window->_toolbox->_temp_tex_color_buffer[ 0 ] )
    glDeleteTextures( 1, &_window->_toolbox->_temp_tex_color_buffer[ 0 ] );
  if( _window->_toolbox->_temp_tex_color_buffer[ 1 ] )
    glDeleteTextures( 1, &_window->_toolbox->_temp_tex_color_buffer[ 1 ] );
  if( _window->_toolbox->_final_tex_color_buffer[ 0 ] )
    glDeleteTextures( 1, &_window->_toolbox->_final_tex_color_buffer[ 0 ] );
  if( _window->_toolbox->_final_tex_color_buffer[ 1 ] )
    glDeleteTextures( 1, &_window->_toolbox->_final_tex_color_buffer[ 1 ] );
  if( _tex_albedo_ground )
    glDeleteTextures( 1, &_tex_albedo_ground );
  if( _tex_normal_ground )
    glDeleteTextures( 1, &_tex_normal_ground );
  if( _tex_height_ground )
    glDeleteTextures( 1, &_tex_height_ground );
  if( _tex_AO_ground )
    glDeleteTextures( 1, &_tex_AO_ground );
  if( _tex_roughness_ground )
    glDeleteTextures( 1, &_tex_roughness_ground );
  if( _tex_metalness_ground )
    glDeleteTextures( 1, &_tex_metalness_ground );


  // Delete VAOs
  // -----------
  if( _lampVAO )
    glDeleteVertexArrays( 1, &_lampVAO );
  if( _groundVAO )
    glDeleteVertexArrays( 1, &_groundVAO );
  

  // Delete VBOs
  // -----------
  if( _lampVBO )
    glDeleteBuffers( 1, &_lampVBO );
  if( _groundVBO )
    glDeleteBuffers( 1, &_groundVBO );


  // Delete FBOs
  // -----------
  if( _window->_toolbox->_temp_hdr_FBO )
    glDeleteFramebuffers( 1, &_window->_toolbox->_temp_hdr_FBO );
  if( _window->_toolbox->_final_hdr_FBO )
    glDeleteFramebuffers( 1, &_window->_toolbox->_final_hdr_FBO );
  if( _window->_toolbox->_temp_hdr_FBO )
    glDeleteFramebuffers( 1, &_window->_toolbox->_temp_hdr_FBO );
  if( _window->_toolbox->_captureFBO )
    glDeleteFramebuffers( 1, &_window->_toolbox->_captureFBO );


  // Delete RBOs
  // -----------
  if( _window->_toolbox->_temp_depht_RBO )
    glDeleteRenderbuffers( 1, &_window->_toolbox->_temp_depht_RBO );
  if( _window->_toolbox->_final_depht_RBO )
    glDeleteRenderbuffers( 1, &_window->_toolbox->_final_depht_RBO );
  if( _window->_toolbox->_captureRBO )
    glDeleteRenderbuffers( 1, &_window->_toolbox->_captureRBO );
}

void Scene::SceneDataInitialization()
{

  float aniso = 0.0f; 
  glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso );

  SDL_Surface * t = NULL;


  // Create all primitive geometry data
  // ----------------------------------

  // Create observer geometry
  GLfloat observer[] =
  {
    -1.0, -1.0, 0.0,
     1.0, -1.0, 0.0,
    -1.0,  1.0, 0.0,  
     1.0,  1.0, 0.0,
    
    ///////////////
      
    0.0f, 0.0f, 1.0f, 0.0f, 
    0.0f, 1.0f, 1.0f, 1.0f     
  };

  // Create Sphere geometry
  GLfloat * dataLamp = _window->_toolbox->BuildSphere( _window->_toolbox->_sphere_longitude_count,
                                                       _window->_toolbox->_sphere_latitude_count );
  _window->_toolbox->_sphere_vertices_count = ( 6 * 3 * _window->_toolbox->_sphere_longitude_count * _window->_toolbox->_sphere_latitude_count );


  // Skybox texture
  _faces.push_back( "../Skybox/s1/front.png" );
  _faces.push_back( "../Skybox/s1/back.png" );
  _faces.push_back( "../Skybox/s1/top.png" );
  _faces.push_back( "../Skybox/s1/bottom.png" );
  _faces.push_back( "../Skybox/s1/right.png" );
  _faces.push_back( "../Skybox/s1/left.png" );


  // Create lamp VAO
  // ---------------
  glGenVertexArrays( 1, &_lampVAO );
  glBindVertexArray( _lampVAO);
  glEnableVertexAttribArray( 0 );
  glEnableVertexAttribArray( 1  );
  glGenBuffers( 1, &_lampVBO );
  glBindBuffer( GL_ARRAY_BUFFER, _lampVBO );
  glBufferData( GL_ARRAY_BUFFER,( ( 6 * 6 * _window->_toolbox->_sphere_longitude_count * _window->_toolbox->_sphere_latitude_count ) ) * ( sizeof( float ) ), dataLamp, GL_STATIC_DRAW );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * ( sizeof( float ) ), ( const void * )0 );
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * ( sizeof( float ) ), /*3*(sizeof(float))*/( const void * )0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );


  // Create ground VAO
  // -----------------
  if( _groundVAO == 0 )
  { 
    // Positions, build 1*1 meter square
    glm::vec3 pos1( -0.5,  0.5, 0.0 );
    glm::vec3 pos2( -0.5, -0.5, 0.0 );
    glm::vec3 pos3(  0.5, -0.5, 0.0 );
    glm::vec3 pos4(  0.5,  0.5, 0.0 );
    
    // texture coordinates
    glm::vec2 uv1( 0.0, 1.0 );
    glm::vec2 uv2( 0.0, 0.0 );
    glm::vec2 uv3( 1.0, 0.0 );
    glm::vec2 uv4( 1.0, 1.0 );
    uv1 *= 0.5 * _ground1->_scale[ 0 ];
    uv2 *= 0.5 * _ground1->_scale[ 0 ];
    uv3 *= 0.5 * _ground1->_scale[ 0 ];
    uv4 *= 0.5 * _ground1->_scale[ 0 ];

    // normal vector
    glm::vec3 nm( 0.0, 0.0, 1.0 );

    // calculate tangent/bitangent vectors of both triangles
    glm::vec3 tangent1, bitangent1;
    glm::vec3 tangent2, bitangent2;

    // - triangle 1
    glm::vec3 edge1 = pos2 - pos1;
    glm::vec3 edge2 = pos3 - pos1;
    glm::vec2 deltaUV1 = uv2 - uv1;
    glm::vec2 deltaUV2 = uv3 - uv1;
    GLfloat f = 1.0f / ( deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y );
    tangent1.x = f * ( deltaUV2.y * edge1.x - deltaUV1.y * edge2.x );
    tangent1.y = f * ( deltaUV2.y * edge1.y - deltaUV1.y * edge2.y );
    tangent1.z = f * ( deltaUV2.y * edge1.z - deltaUV1.y * edge2.z );
    tangent1 = glm::normalize( tangent1 );
    bitangent1.x = f * ( -deltaUV2.x * edge1.x + deltaUV1.x * edge2.x );
    bitangent1.y = f * ( -deltaUV2.x * edge1.y + deltaUV1.x * edge2.y );
    bitangent1.z = f * ( -deltaUV2.x * edge1.z + deltaUV1.x * edge2.z );
    bitangent1 = glm::normalize( bitangent1 );

    // - triangle 2
    edge1 = pos3 - pos1;
    edge2 = pos4 - pos1;
    deltaUV1 = uv3 - uv1;
    deltaUV2 = uv4 - uv1;
    f = 1.0f / ( deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y );
    tangent2.x = f * ( deltaUV2.y * edge1.x - deltaUV1.y * edge2.x );
    tangent2.y = f * ( deltaUV2.y * edge1.y - deltaUV1.y * edge2.y );
    tangent2.z = f * ( deltaUV2.y * edge1.z - deltaUV1.y * edge2.z );
    tangent2 = glm::normalize( tangent2 );
    bitangent2.x = f * ( -deltaUV2.x * edge1.x + deltaUV1.x * edge2.x );
    bitangent2.y = f * ( -deltaUV2.x * edge1.y + deltaUV1.x * edge2.y );
    bitangent2.z = f * ( -deltaUV2.x * edge1.z + deltaUV1.x * edge2.z );
    bitangent2 = glm::normalize( bitangent2 );

    //std::cout << "TEST = " << bitangent1.x << ", " << bitangent1.y << ", " << bitangent1.z << std::endl;

    GLfloat groundVertices[] =
    {
      // Positions            // normal         // TexCoords  // Tangent                          // Bitangent
      pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
      pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
      pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

      pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
      pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
      pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
    };
    
    // Setup plane VAO
    glGenVertexArrays( 1, &_groundVAO );
    glGenBuffers( 1, &_groundVBO );
    glBindVertexArray( _groundVAO );
    glBindBuffer( GL_ARRAY_BUFFER, _groundVBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( groundVertices ), &groundVertices, GL_STATIC_DRAW );
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
  }


  // Create temp color buffer
  // ------------------------
  glGenFramebuffers( 1, &_window->_toolbox->_temp_hdr_FBO );
  glGenRenderbuffers( 1, &_window->_toolbox->_temp_depht_RBO );
  glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_temp_hdr_FBO );
  glGenTextures( 2, _window->_toolbox->_temp_tex_color_buffer );

  if( _multi_sample )
  {
    for( GLuint i = 0; i < 2; i++ ) 
    {
      glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, _window->_toolbox->_temp_tex_color_buffer[ i ] );
      glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, _nb_multi_sample , GL_RGB16F, _window->_width, _window->_height, GL_TRUE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );  
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, _window->_toolbox->_temp_tex_color_buffer[ i ], 0 );
    }
    glBindRenderbuffer( GL_RENDERBUFFER, _window->_toolbox->_temp_depht_RBO );
    glRenderbufferStorageMultisample( GL_RENDERBUFFER, _nb_multi_sample, GL_DEPTH24_STENCIL8, _window->_width, _window->_height ); 
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _window->_toolbox->_temp_depht_RBO );

    GLuint attachments2[ 2 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers( 2, attachments2 );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
      std::cout << "Framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  }
  else
  {
    for( GLuint i = 0; i < 2; i++ ) 
    {
      glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_temp_tex_color_buffer[ i ] );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, _window->_width, _window->_height, 0, GL_RGB, GL_FLOAT, NULL );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );  
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _window->_toolbox->_temp_tex_color_buffer[ i ], 0 );
    }
    glBindRenderbuffer( GL_RENDERBUFFER, _window->_toolbox->_temp_depht_RBO );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _window->_width, _window->_height );

    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _window->_toolbox->_temp_depht_RBO );
    GLuint attachments2[ 2 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers( 2, attachments2 );
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
      std::cout << "Framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );    
  }


  // Create final color buffer
  // -------------------------
  glGenFramebuffers( 1, &_window->_toolbox->_final_hdr_FBO );
  glGenRenderbuffers( 1, &_window->_toolbox->_final_depht_RBO );
  glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_final_hdr_FBO );
  glGenTextures( 2, _window->_toolbox->_final_tex_color_buffer);

  for( GLuint i = 0; i < 2; i++ ) 
  {
    glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_final_tex_color_buffer[ i ] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, _window->_width, _window->_height, 0, GL_RGB, GL_FLOAT, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );  
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _window->_toolbox->_final_tex_color_buffer[ i ], 0 );
  }
  glBindRenderbuffer( GL_RENDERBUFFER, _window->_toolbox->_final_depht_RBO );
  glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _window->_width, _window->_height );
  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _window->_toolbox->_final_depht_RBO );
  glBindRenderbuffer( GL_RENDERBUFFER, 0 );
  if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
  {
    std::cout << "Framebuffer not complete!" << std::endl;
  }
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glBindTexture( GL_TEXTURE_2D, 0 );


  // Create pingpong buffer
  // ----------------------
  glGenFramebuffers( 2, _window->_toolbox->_pingpong_FBO );
  glGenTextures( 2, _window->_toolbox->_pingpong_color_buffers );
  for( GLuint i = 0; i < 2; i++ )
  {
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_pingpong_FBO[ i ] );
    glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_pingpong_color_buffers[ i ] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, _window->_width * _bloom_downsample, _window->_height * _bloom_downsample, 0, GL_RGB, GL_FLOAT, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _window->_toolbox->_pingpong_color_buffers[ i ], 0 );
    if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
      std::cout << "Framebuffer not complete!" << std::endl;
    }
  }
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glBindTexture( GL_TEXTURE_2D, 0 );
    

  // Load ground albedo texture
  // --------------------------
  glGenTextures( 1, &_tex_albedo_ground );
  glBindTexture( GL_TEXTURE_2D, _tex_albedo_ground );
  if( ( t = IMG_Load( "../Textures/ground1/albedo.png" ) ) != NULL )
  {
    if( _window->_toolbox->IsTextureRGBA( t ) )
    {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels );
    }
    else
    {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels );
    }
    SDL_FreeSurface( t );
  }
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
  }
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso ); // anisotropie
  glGenerateMipmap( GL_TEXTURE_2D );
  glBindTexture( GL_TEXTURE_2D, 0 );


  // Load ground normal texture
  // ----------------------------
  glGenTextures( 1, &_tex_normal_ground );
  glBindTexture( GL_TEXTURE_2D, _tex_normal_ground );
  if( ( t = IMG_Load( "../Textures/ground1/normal.png" ) ) != NULL )
  {
    if( _window->_toolbox->IsTextureRGBA( t ) )
    {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels );
    }
    else
    {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels );
    }
    SDL_FreeSurface( t );
  } 
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
  }
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso ); // anisotropie
  glGenerateMipmap( GL_TEXTURE_2D );
  glBindTexture( GL_TEXTURE_2D, 0 );


  // Load ground height texture
  // --------------------------
  glGenTextures( 1, &_tex_height_ground );
  glBindTexture( GL_TEXTURE_2D, _tex_height_ground );
  if( ( t = IMG_Load( "../Textures/ground1/height.png" ) ) != NULL )
  {
    if( _window->_toolbox->IsTextureRGBA( t ) )
    {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels );
    }
    else
    {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels );
    }
    SDL_FreeSurface( t );
  } 
  else 
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
  }
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso ); // anisotropie
  glGenerateMipmap( GL_TEXTURE_2D );
  glBindTexture( GL_TEXTURE_2D, 0 );


  // Load ground AO texture
  // ----------------------
  glGenTextures( 1, &_tex_AO_ground );
  glBindTexture( GL_TEXTURE_2D, _tex_AO_ground );
  if( ( t = IMG_Load( "../Textures/ground1/AO.png" ) ) != NULL ) 
  {
    if( _window->_toolbox->IsTextureRGBA( t ) )
    {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels );
    }
    else
    {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels );
    }
    SDL_FreeSurface( t );
  } 
  else 
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
  }
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso ); // anisotropie
  glGenerateMipmap( GL_TEXTURE_2D );
  glBindTexture( GL_TEXTURE_2D, 0 );


  // Load ground roughness texture 
  // -----------------------------
  glGenTextures( 1, &_tex_roughness_ground );
  glBindTexture( GL_TEXTURE_2D, _tex_roughness_ground );
  if( ( t = IMG_Load( "../Textures/ground1/roughness.png" ) ) != NULL ) 
  {
    if( _window->_toolbox->IsTextureRGBA( t ) )
    {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels );
    }
    else
    {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels );
    }
    SDL_FreeSurface( t );
  } 
  else 
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
  }
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso ); // anisotropie
  glGenerateMipmap( GL_TEXTURE_2D );
  glBindTexture( GL_TEXTURE_2D, 0 );


  // Load ground metalness texture  
  // -----------------------------
  glGenTextures( 1, &_tex_metalness_ground );
  glBindTexture( GL_TEXTURE_2D, _tex_metalness_ground );
  if( ( t = IMG_Load( "../Textures/ground1/metalness.png" ) ) != NULL ) 
  {
   if( _window->_toolbox->IsTextureRGBA( t ) )
   {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels );
    }
    else
    {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels );
    }
    SDL_FreeSurface( t );
  } 
  else 
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
  }
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso ); // anisotropie
  glGenerateMipmap( GL_TEXTURE_2D );
  glBindTexture( GL_TEXTURE_2D, 0 );

}

void Scene::LightsInitialization()
{ 
  _lights.clear();
  
  Light::SetLightsMultiplier( 100.0 );

  for( int i = 0; i < 10; i++ )
  {
    _lights.push_back( Light ( glm::vec3( -5.0 + ( i * 1.0 ), 0.5, 0 ),
                               glm::vec3( 1.0, 1.0, 1.0 ),
                               0.1 ) );    
  }

  for( int i = 0; i < _lights.size(); i++ )
  {
    _lights[ i ]._intensity *= Light::GetLightsMultiplier();
  }
}

void Scene::ObjectsInitialization()
{
  glm::vec3 position;


  // Tables object initialization
  // ----------------------------
  for( int i = 0; i < 3; i++ )
  { 

    switch( i )
    {
      case 0:
        position = glm::vec3( -1.5, 1.0, 2.0 );
        break;
      
      case 1:
        position = glm::vec3( 0.0, 1.0, 2.0 );
        break;

      case 2:
        position = glm::vec3( 1.5, 1.0, 2.0 );
        break;
    }

    _tables.push_back( Object( 0, // ID
                               position,
                               0.0,
                               0.0,
                               glm::vec3( 0.01, 0.01, 0.01 ),
                               1.0,   // alpha
                               8,
                               false,
                               0.75,
                               true,   // normal mapping
                               false,   // bloom
                               1.0 ) );
  }


  // _Ground1 object initialization
  // ------------------------------
  _ground1 = new Object( 1, // ID
                         glm::vec3( 0.0, 0.0, 0.0 ),
                         _PI_2,
                         0.0,
                         glm::vec3( 10.0, 10.0, 1.0 ),
                         1.0,
                         8,
                         false,
                         0.75,
                         true,
                         false,
                         1.0 );

}

void Scene::IBLCubeMapsInitialization()
{ 
  std::vector< std::string > hdr_texture_paths;
  hdr_texture_paths.push_back( std::string( "../Skybox/hdr skybox 2/Ridgecrest_Road_Ref.hdr" ) );
  hdr_texture_paths.push_back( std::string( "../Skybox/hdr skybox 1/Arches_E_PineTree_3k.hdr" ) );
  hdr_texture_paths.push_back( std::string( "../Skybox/hdr skybox 3/QueenMary_Chimney_Ref.hdr" ) );

  for( unsigned int i = 0; i < hdr_texture_paths.size(); i++ )
  {

    unsigned int hdr_texture;
    unsigned int env_cubemap;
    unsigned int irradiance_map;

    // Load hdr skybox texture
    _window->_toolbox->_hdr_image_manager->stbi_set_flip_vertically_on_load( true );
    int width, height, nrComponents;
    
    float * data = _window->_toolbox->_hdr_image_manager->stbi_loadf( hdr_texture_paths[ i ].c_str(), &width, &height, &nrComponents, 0 );
    if( data )
    {
      glGenTextures( 1, &hdr_texture );
      glBindTexture( GL_TEXTURE_2D, hdr_texture );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data ); // note how we specify the texture's data value to be float
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      _window->_toolbox->_hdr_image_manager->stbi_image_free( data );
    }
    else
    {
      std::cout << "Failed to load HDR image.\n" << std::endl;
    }

    // Gen FBO and RBO to render hdr tex into cube map tex
    glGenFramebuffers( 1, &_window->_toolbox->_captureFBO );
    glGenRenderbuffers( 1, &_window->_toolbox->_captureRBO );
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_captureFBO );
    glBindRenderbuffer( GL_RENDERBUFFER, _window->_toolbox->_captureRBO );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _res_IBL_cubeMap, _res_IBL_cubeMap );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _window->_toolbox->_captureRBO );

    // Gen cubemap texture
    glGenTextures( 1, &env_cubemap );
    glBindTexture( GL_TEXTURE_CUBE_MAP, env_cubemap );
    for( unsigned int i = 0; i < 6; ++i )
    {
      glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, _res_IBL_cubeMap, _res_IBL_cubeMap, 0, GL_RGB, GL_FLOAT, nullptr );
    }
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // create 6 matrix to each cube map face
    glm::mat4 captureProjection = glm::perspective( glm::radians( 90.0f ), 1.0f, 0.1f, 10.0f );
    glm::mat4 captureViews[] =
    {
      glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  1.0f,  0.0f,  0.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) ),
      glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( -1.0f,  0.0f,  0.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) ),
      glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  0.0f,  1.0f,  0.0f ), glm::vec3( 0.0f,  0.0f,  1.0f ) ),
      glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  0.0f, -1.0f,  0.0f ), glm::vec3( 0.0f,  0.0f, -1.0f ) ),
      glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  0.0f,  0.0f,  1.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) ),
      glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(  0.0f,  0.0f, -1.0f ), glm::vec3( 0.0f, -1.0f,  0.0f ) )
    };

    // Convert skybox hdr texture into cube map texture
    _cube_map_converter_shader.Use();
    glUniform1i(glGetUniformLocation( _cube_map_converter_shader._program, "uEquirectangularMap" ), 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, hdr_texture );
    glUniformMatrix4fv( glGetUniformLocation( _cube_map_converter_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( captureProjection ) );

    glViewport( 0, 0, _res_IBL_cubeMap, _res_IBL_cubeMap ); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_captureFBO );
    for( unsigned int i = 0; i < 6; ++i )
    {
      glUniformMatrix4fv( glGetUniformLocation( _cube_map_converter_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( captureViews[ i ] ) );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, env_cubemap, 0 );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      _window->_toolbox->RenderCube();
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    // Gen irradiance cube map texture
    glGenTextures( 1, &irradiance_map );
    glBindTexture( GL_TEXTURE_CUBE_MAP, irradiance_map );
    for( unsigned int i = 0; i < 6; ++i )
    {
      glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0,
                    GL_RGB32F,
                    _res_irradiance_cubeMap,
                    _res_irradiance_cubeMap,
                    0,
                    GL_RGB,
                    GL_FLOAT, 
                    nullptr );
    }
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_captureFBO );
    glBindRenderbuffer( GL_RENDERBUFFER, _window->_toolbox->_captureRBO );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _res_irradiance_cubeMap, _res_irradiance_cubeMap );

    // Diffuse irradiance cube map calculation  
    _diffuse_irradiance_shader.Use();
    glUniformMatrix4fv( glGetUniformLocation( _diffuse_irradiance_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( captureProjection ) );
    glUniform1f( glGetUniformLocation( _diffuse_irradiance_shader._program, "uSampleDelta" ), _irradiance_sample_delta );
    glUniform1i( glGetUniformLocation( _diffuse_irradiance_shader._program, "uEnvironmentMap" ), 0 );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, env_cubemap );
    
    glViewport( 0, 0, _res_irradiance_cubeMap, _res_irradiance_cubeMap ); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_captureFBO );
    for( unsigned int i = 0; i < 6; ++i )
    {
      glUniformMatrix4fv( glGetUniformLocation( _diffuse_irradiance_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( captureViews[ i ] ) );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_map, 0 );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      _window->_toolbox->RenderCube();
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    // save texture IDs
    _hdr_textures.push_back( hdr_texture );
    _env_cubemaps.push_back( env_cubemap);
    _irradiance_maps.push_back( irradiance_map );
  }
}

void Scene::ShadersInitialization()
{

  // Compilation des shaders
  // -----------------------
  _forward_pbr_shader.SetShaderClassicPipeline(        "../Shaders/forward_pbr_lighting.vs",   "../Shaders/forward_pbr_lighting.fs" );
  _skybox_shader.SetShaderClassicPipeline(             "../Shaders/skybox.vs",                 "../Shaders/skybox.fs" );
  _flat_color_shader.SetShaderClassicPipeline(         "../Shaders/flat_color.vs",             "../Shaders/flat_color.fs" );
  _observer_shader.SetShaderClassicPipeline(           "../Shaders/observer.vs",               "../Shaders/observer.fs" );
  _blur_shader.SetShaderClassicPipeline(               "../Shaders/post_process.vs",           "../Shaders/blur.fs" );
  _post_process_shader.SetShaderClassicPipeline(       "../Shaders/post_process.vs",           "../Shaders/post_process.fs" );
  _blit_shader.SetShaderClassicPipeline(               "../Shaders/multisample_blit.vs",       "../Shaders/multisample_blit.fs" );
  _cube_map_converter_shader.SetShaderClassicPipeline( "../Shaders/cube_map_converter.vs",     "../Shaders/cube_map_converter.fs" );
  _diffuse_irradiance_shader.SetShaderClassicPipeline( "../Shaders/cube_map_converter.vs",     "../Shaders/diffuse_irradiance.fs" );
  _geometry_pass_shader.SetShaderClassicPipeline(      "../Shaders/deferred_geometry_pass.vs", "../Shaders/deferred_geometry_pass.fs" );
  _lighting_pass_shader.SetShaderClassicPipeline(      "../Shaders/deferred_lighting_pass.vs", "../Shaders/deferred_lighting_pass.fs" );


  // Set texture uniform location
  // ----------------------------
  _forward_pbr_shader.Use();
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureDiffuse1" ), 0 ) ;
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureNormal1" ), 1 );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureHeight1" ), 2 ) ;
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureAO1" ), 3 );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureRoughness1" ), 4 );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureMetalness1" ), 5 );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureSpecular1" ), 6 );

  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIrradianceCubeMap" ), 9 ); 
  glUseProgram( 0 );

  _geometry_pass_shader.Use();
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureDiffuse1" ), 0 ) ;
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureNormal1" ), 1 );
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureHeight1" ), 2 ) ;
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureAO1" ), 3 );
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureRoughness1" ), 4 );
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureMetalness1" ), 5 );
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureSpecular1" ), 6 );
  glUseProgram( 0 );

  _lighting_pass_shader.Use();
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uGbufferPosition" ), 0 ) ;
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uGbufferNormal" ), 1 );
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uGbufferAlbedo" ), 2 ) ;
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uGbufferRougnessMetalnessAO" ), 3 );
  glUseProgram( 0 );

  _skybox_shader.Use();
  glUniform1i( glGetUniformLocation( _skybox_shader._program, "uSkyboxTexture" ), 0 );
  glUseProgram( 0 );

  _observer_shader.Use();
  glUniform1i( glGetUniformLocation( _observer_shader._program, "uTexture1" ), 0 );
  glUseProgram( 0 );

  _blit_shader.Use();
  glUniform1i( glGetUniformLocation( _blit_shader._program, "uTexture1" ), 0 );
  glUseProgram( 0 );

  _blur_shader.Use();
  glUniform1i( glGetUniformLocation( _blur_shader._program, "uTexture" ), 0 );
  glUseProgram( 0 );

  _post_process_shader.Use();
  glUniform1i( glGetUniformLocation( _post_process_shader._program, "uBaseColorTexture" ), 0 );
  glUniform1i( glGetUniformLocation( _post_process_shader._program, "uBloomBrightnessTexture" ), 1 );
  glUseProgram( 0 );
}

void Scene::ModelsLoading()
{ 
  Model::SetToolbox( _window->_toolbox );
  // Table model loading
  _table_model = new Model();
  _table_model->Load_Model( "../Models/cube/Rounded Cube.fbx", 0, "Table1" );
  _table_model->Print_info_model();
}

void Scene::DeferredBuffersInitialization()
{

  // G buffer initialization
  // -----------------------
  glGenFramebuffers( 1, &_g_buffer_FBO );
  glBindFramebuffer( GL_FRAMEBUFFER, _g_buffer_FBO );

  GLuint texture_id;
  for( unsigned int i = 0; i < 4; i++ )
  {
    glGenTextures( 1, &texture_id );
    glBindTexture( GL_TEXTURE_2D, texture_id );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, _window->_width, _window->_height, 0, GL_RGB, GL_FLOAT, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texture_id, 0 );
    _g_buffer_textures.push_back( texture_id );
  }

  unsigned int attachments[ 4 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
  glDrawBuffers( 4, attachments );

  glGenRenderbuffers( 1, &_g_buffer_RBO );
  glBindRenderbuffer( GL_RENDERBUFFER, _g_buffer_RBO );
  glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _window->_width, _window->_height );
  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _g_buffer_RBO );
    
  if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
  {
    std::cout << "ERROR : G-buffer's FBO not complete" << std::endl;
  }
    
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void Scene::SceneForwardRendering( bool iIsFinalFBO )
{

  // Matrices setting
  // ----------------
  glm::mat4 projectionM, Msend, viewMatrix;

  projectionM = glm::perspective( 45.0f, ( float )_window->_width / ( float )_window->_height, _camera->_near, _camera->_far );
  viewMatrix = glm::lookAt( _camera->_position, _camera->_position + _camera->_front, _camera->_up ); 

  glm::mat4 lightProjection, lightView, light_space_matrix, skybox_light_space_matrix;


  // Bind correct buffer for drawing
  // -------------------------------
  if( iIsFinalFBO )
  {
    if( _multi_sample )
    {
      glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_temp_hdr_FBO /*final_hdr_FBO*/ );
    }
    else{
      glBindFramebuffer( GL_FRAMEBUFFER, /*final_hdr_FBO*/ _window->_toolbox->_temp_hdr_FBO );
    }
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  }


  // Draw skybox
  // -----------
  glDepthMask( GL_FALSE ); // desactivé juste pour draw la skybox
  _skybox_shader.Use();   
  glm::mat4 SkyboxViewMatrix = glm::mat4( glm::mat3( viewMatrix ) );  // Remove any translation component of the view matrix

  Msend = glm::mat4( 1.0f );
  glUniformMatrix4fv( glGetUniformLocation( _skybox_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( projectionM ) );
  glUniformMatrix4fv( glGetUniformLocation( _skybox_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( SkyboxViewMatrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _skybox_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( Msend ) );
  glUniform1f( glGetUniformLocation( _skybox_shader._program, "uAlpha" ), 1.0 );

  glUniform1i( glGetUniformLocation( _skybox_shader._program, "uBloom" ), false );
  glUniform1f( glGetUniformLocation( _skybox_shader._program, "uBloomBrightness" ), _ground1->_bloom_brightness );

  glActiveTexture( GL_TEXTURE0 );
  //glBindTexture( GL_TEXTURE_CUBE_MAP, _irradiance_maps[ _current_env ] ); // bind les 6 textures du cube map 
  glBindTexture( GL_TEXTURE_CUBE_MAP, _env_cubemaps[ _current_env ] ); // bind les 6 textures du cube map 

  glEnable( GL_BLEND );
  _window->_toolbox->RenderCube();
  glDisable( GL_BLEND );

  glDepthMask( GL_TRUE );  // réactivé pour draw le reste
  glUseProgram( 0 );


  // Draw lamps
  // ----------
  _flat_color_shader.Use();
  glBindVertexArray( _lampVAO );

  for( int i = 0; i < _lights.size(); i++ )
  {
    Msend= glm::mat4();
    Msend = glm::translate( Msend, _lights[ i ]._position );
    Msend = glm::scale( Msend, glm::vec3( 0.06f ) ); 
    glm::vec3 lampColor = _lights[ i ]._color * _lights[ i ]._intensity;
    glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uViewMatrix" ) , 1, GL_FALSE, glm::value_ptr( viewMatrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( Msend ) );
    glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( projectionM ) );
    glUniform3f( glGetUniformLocation( _flat_color_shader._program, "uColor" ), lampColor.x, lampColor.y, lampColor.z );

    glUniform1i( glGetUniformLocation( _flat_color_shader._program, "uBloom" ), true );
    glUniform1f( glGetUniformLocation( _flat_color_shader._program, "uBloomBrightness" ), 1.0f );

    glDrawArrays( GL_TRIANGLES, 0, _window->_toolbox->_sphere_vertices_count );
  }
  glBindVertexArray( 0 );
  glUseProgram( 0 );


  // Draw ground1
  // ------------
  _forward_pbr_shader.Use();

  Msend = glm::mat4();

  Msend = glm::translate( Msend, _ground1->_position );
  Msend = glm::rotate( Msend, _ground1->_angle, glm::vec3( -1.0, 0.0 , 0.0 ) );
  Msend = glm::scale( Msend, _ground1->_scale ); 

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, _tex_albedo_ground );  
  glActiveTexture( GL_TEXTURE1 );
  glBindTexture( GL_TEXTURE_2D, _tex_normal_ground ); 
  glActiveTexture( GL_TEXTURE2 );
  glBindTexture( GL_TEXTURE_2D, _tex_height_ground ); 
  glActiveTexture( GL_TEXTURE3 );
  glBindTexture( GL_TEXTURE_2D, _tex_AO_ground ); 
  glActiveTexture( GL_TEXTURE4 );
  glBindTexture( GL_TEXTURE_2D, _tex_roughness_ground ); 
  glActiveTexture( GL_TEXTURE5 );
  glBindTexture( GL_TEXTURE_2D, _tex_metalness_ground ); 

  glActiveTexture( GL_TEXTURE9 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, _irradiance_maps[ _current_env ] /*envCubemap*/ ); // bind les 6 textures du cube map 

  glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( viewMatrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( Msend ) );
  glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( projectionM ) );

  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightCount" ), _lights.size() );

  for( int i = 0; i < _lights.size(); i++ )
  {
    string temp = to_string( i );
    glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, ( "uLightPos[" + temp + "]" ).c_str() ),1, &_lights[ i ]._position[ 0 ] );
    glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, ( "uLightColor[" + temp + "]" ).c_str() ),1, &_lights[ i ]._color[ 0 ] );
    glUniform1f(  glGetUniformLocation( _forward_pbr_shader._program, ( "uLightIntensity[" + temp + "]" ).c_str() ), _lights[ i ]._intensity );
  }

  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _ground1->_bloom );
  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _ground1->_bloom_brightness );

  glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewPos" ), 1, &_camera->_position[ 0 ] );

  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _ground1->_alpha );
  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _ground1->_id );    
  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uCubeMapFaceNum" ), -1.0 );     

  glBindVertexArray( _groundVAO );
  glDrawArrays( GL_TRIANGLES, 0, 6 );
  glBindVertexArray( 0 );
  glUseProgram( 0 );
 

  // Draw tables
  // -----------
  _forward_pbr_shader.Use();

  for( int i = 0; i < _tables.size(); i++ )
  {
    Msend = glm::mat4();

    Msend = glm::translate( Msend, _tables[ i ]._position );
    Msend = glm::rotate( Msend, _tables[ i ]._angle, glm::vec3( -1.0, 0.0 , 0.0 ) );
    Msend = glm::scale( Msend, _tables[ i ]._scale ); 

    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _irradiance_maps[ _current_env ] ); 

    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( viewMatrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( Msend ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( projectionM ) );

    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightCount" ), _lights.size() );

    for( int i = 0; i < _lights.size(); i++ )
    {
      string temp = to_string( i );
      glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, ( "uLightPos[" + temp + "]" ).c_str() ),1, &_lights[ i ]._position[ 0 ] );
      glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, ( "uLightColor[" + temp + "]" ).c_str() ),1, &_lights[ i ]._color[ 0 ] );
      glUniform1f(  glGetUniformLocation( _forward_pbr_shader._program, ( "uLightIntensity[" + temp + "]" ).c_str() ), _lights[ i ]._intensity );
    }

    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _tables[ i ]._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _tables[ i ]._bloom_brightness );

    glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewPos" ), 1, &_camera->_position[ 0 ] );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _tables[ i ]._alpha );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _tables[ i ]._id );    
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uCubeMapFaceNum" ), -1.0 ); 

    _table_model->Draw( _forward_pbr_shader );
  } 
  glUseProgram( 0 );

  // Unbinding    
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glBindVertexArray( 0 );


  // Blit multi sample texture to classic texture
  // --------------------------------------------
  if( _multi_sample )
  {
    _blit_shader.Use();

    // Bind classic texture where we want to render
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_final_hdr_FBO );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _window->_toolbox->_final_tex_color_buffer[ 0 ], 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Send multi sample texture we need to convert into classic texture
    glActiveTexture( GL_TEXTURE0) ;
    glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, _window->_toolbox->_temp_tex_color_buffer[ 0 ] );
    glUniform1i( glGetUniformLocation( _blit_shader._program, "uSampleCount" ), _nb_multi_sample );    
    _window->_toolbox->RenderQuad();

    // Same convert with brightness texture ( bloom )
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_final_hdr_FBO );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _window->_toolbox->_final_tex_color_buffer[ 1 ], 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, _window->_toolbox->_temp_tex_color_buffer[ 1 ] );
    glUniform1i( glGetUniformLocation( _blit_shader._program, "uSampleCount" ), _nb_multi_sample );
    _window->_toolbox->RenderQuad();
  }
 
  // Unbinding    
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glBindVertexArray( 0 );
  glUseProgram( 0 );
}

void Scene::DeferredGeometryPass()
{ 

  // Matrices setting
  // ----------------
  glm::mat4 projection_matrix, view_matrix, model_matrix;
  projection_matrix = glm::perspective( 45.0f, ( float )_window->_width / ( float )_window->_height, _camera->_near, _camera->_far );
  view_matrix = glm::lookAt( _camera->_position, _camera->_position + _camera->_front, _camera->_up ); 


  // Bind G buffer
  glBindFramebuffer( GL_FRAMEBUFFER, _g_buffer_FBO );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


  // Draw ground1
  // ------------
  _geometry_pass_shader.Use();

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, _ground1->_position );
  model_matrix = glm::rotate( model_matrix, _ground1->_angle, glm::vec3( -1.0, 0.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, _ground1->_scale ); 

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, _tex_albedo_ground );  
  glActiveTexture( GL_TEXTURE1 );
  glBindTexture( GL_TEXTURE_2D, _tex_normal_ground ); 
  glActiveTexture( GL_TEXTURE3 );
  glBindTexture( GL_TEXTURE_2D, _tex_AO_ground ); 
  glActiveTexture( GL_TEXTURE4 );
  glBindTexture( GL_TEXTURE_2D, _tex_roughness_ground ); 
  glActiveTexture( GL_TEXTURE5 );
  glBindTexture( GL_TEXTURE_2D, _tex_metalness_ground ); 

  glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( view_matrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( projection_matrix ) );

  glBindVertexArray( _groundVAO );
  glDrawArrays( GL_TRIANGLES, 0, 6 );
  glBindVertexArray( 0 );
  glUseProgram( 0 );


  // Draw tables
  // -----------
  _geometry_pass_shader.Use();

  for( int i = 0; i < _tables.size(); i++ )
  {
    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, _tables[ i ]._position );
    model_matrix = glm::rotate( model_matrix, _tables[ i ]._angle, glm::vec3( -1.0, 0.0 , 0.0 ) );
    model_matrix = glm::scale( model_matrix, _tables[ i ]._scale ); 

    glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( projection_matrix ) );

    _table_model->Draw( _geometry_pass_shader );
  } 
  glUseProgram( 0 );

  // Unbinding    
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glBindVertexArray( 0 );
}

void Scene::DeferredLightingPass()
{   
  // Bind correct FBO
  glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_temp_hdr_FBO );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  _lighting_pass_shader.Use();   
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, _g_buffer_textures[ 0 ] );  
  glActiveTexture( GL_TEXTURE1 );
  glBindTexture( GL_TEXTURE_2D, _g_buffer_textures[ 1 ] ); 
  glActiveTexture( GL_TEXTURE3 );
  glBindTexture( GL_TEXTURE_2D, _g_buffer_textures[ 2 ] ); 
  glActiveTexture( GL_TEXTURE4 );
  glBindTexture( GL_TEXTURE_2D, _g_buffer_textures[ 3 ] ); 

  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uLightCount" ), _lights.size() );

  for( int i = 0; i < _lights.size(); i++ )
  {
    string temp = to_string( i );
    glUniform3fv( glGetUniformLocation( _lighting_pass_shader._program, ( "uLightPos[" + temp + "]" ).c_str() ),1, &_lights[ i ]._position[ 0 ] );
    glUniform3fv( glGetUniformLocation( _lighting_pass_shader._program, ( "uLightColor[" + temp + "]" ).c_str() ),1, &_lights[ i ]._color[ 0 ] );
    glUniform1f(  glGetUniformLocation( _lighting_pass_shader._program, ( "uLightIntensity[" + temp + "]" ).c_str() ), _lights[ i ]._intensity );
  }

  glUniform3fv( glGetUniformLocation( _lighting_pass_shader._program, "uViewPos" ), 1, &_camera->_position[ 0 ] );
  
  _window->_toolbox->RenderQuad();
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);     
  glBindVertexArray( 0 );
  glUseProgram( 0 );
}

void Scene::SceneDeferredRendering()
{

  // Perform the deffered geometry pass
  // ----------------------------------
  DeferredGeometryPass();


  // Perform the deffered lighting pass
  // ----------------------------------
  DeferredLightingPass();

}

void Scene::BlurProcess()
{ 
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );   

  bool first_ite = true;
  int horizontal = 1; 
  GLuint amount = 6;
  _blur_shader.Use();
  
  for( GLuint i = 0; i < amount; i++ )
  {
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_pingpong_FBO[ horizontal ] ); 
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    horizontal = ( horizontal == 0 ) ? 1 : 0;

    if( !first_ite )
    {
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_pingpong_color_buffers[ horizontal ] );
    }
    else
    {
      first_ite = false;
      glActiveTexture( GL_TEXTURE0 );
      
      if( _multi_sample )
      {
        glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_final_tex_color_buffer[ 1 ] );    
      }
      else
      {
        glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_temp_tex_color_buffer[ 1 ] ); 
      }
    }

    glUniform1f( glGetUniformLocation( _blur_shader._program, "uHorizontal" ), horizontal );
    glUniform1f( glGetUniformLocation( _blur_shader._program, "uOffsetFactor" ), 1.2 );
    _window->_toolbox->RenderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);     
  }

  glUseProgram(0);
}

void Scene::PostProcess()
{
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  _post_process_shader.Use();

  glActiveTexture( GL_TEXTURE0 );
  
  if( _multi_sample )
  {
    glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_final_tex_color_buffer[ 0 ] );
  }
  else
  {
    glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_temp_tex_color_buffer[ 0 ] );
  }

  if( _bloom )
  {
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_pingpong_color_buffers[ 0 ] );
  }

  glUniform1i( glGetUniformLocation( _post_process_shader._program, "uBloom" ), _bloom );
  glUniform1f( glGetUniformLocation( _post_process_shader._program, "uExposure" ), _exposure );
  _window->_toolbox->RenderQuad();
}