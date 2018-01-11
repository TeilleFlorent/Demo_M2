#include "scene.hpp"
#include "window.hpp"


//******************************************************************************
//**********  Class Scene  *****************************************************
//******************************************************************************

Scene::Scene( Window * iParentWindow )
{ 
  _pipeline_type = FORWARD_RENDERING;
  //_pipeline_type = DEFERRED_RENDERING;


  // Scene effects settings
  // ----------------------
  
  // Frame exposure
  _exposure           = 1.0;

  // Init bloom param
  _bloom              = true;
  _blur_downsample    = 0.5;
  _blur_pass_count    = 6;
  _blur_offset_factor = 0.5;

  // Init multi sample param
  _multi_sample    = false;
  _nb_multi_sample = 2;

  // Init IBL param
  _current_env             = 2;
  _res_env_cubeMap         = 512;

  _res_irradiance_cubeMap  = 32;
  _irradiance_sample_delta = 0.025;
  
  _res_pre_filter_cubeMap  = 256;
  _pre_filter_sample_count = 1024 * 2;
  
  // Lights volume
  _render_lights_volume = false;


  // Scene data initialization
  // -------------------------

  _ground_VAO = 0;
  _ground_VBO = 0;

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
  _camera = new Camera( glm::vec3( -7.02172, 2.42351, -6.27063 ),
                        glm::vec3( 0.724498, -0.409127, 0.554724 ),
                        glm::vec3( 0.0f, 1.0f,  0.0f ),
                        37.44,
                        -24.0,
                        0.01,
                        100.0,
                        45.0f,
                        ( float )_window->_width,
                        ( float )_window->_height,
                        3.0 );

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
    if( _env_cubeMaps[ i ] )
      glDeleteTextures( 1, &_env_cubeMaps[ i ] );
    if( _irradiance_cubeMaps[ i ] )
      glDeleteTextures( 1, &_irradiance_cubeMaps[ i ] );
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
  if( _ground_VAO )
    glDeleteVertexArrays( 1, &_ground_VAO );
  

  // Delete VBOs
  // -----------
  if( _ground_VBO )
    glDeleteBuffers( 1, &_ground_VBO );


  // Delete FBOs
  // -----------
  if( _window->_toolbox->_temp_hdr_FBO )
    glDeleteFramebuffers( 1, &_window->_toolbox->_temp_hdr_FBO );
  if( _window->_toolbox->_final_hdr_FBO )
    glDeleteFramebuffers( 1, &_window->_toolbox->_final_hdr_FBO );
  if( _window->_toolbox->_capture_FBO )
    glDeleteFramebuffers( 1, &_window->_toolbox->_capture_FBO );


  // Delete RBOs
  // -----------
  if( _window->_toolbox->_temp_depth_RBO )
    glDeleteRenderbuffers( 1, &_window->_toolbox->_temp_depth_RBO );
  if( _window->_toolbox->_capture_RBO )
    glDeleteRenderbuffers( 1, &_window->_toolbox->_capture_RBO );
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

  // Skybox texture
  _faces.push_back( "../Skybox/s1/front.png" );
  _faces.push_back( "../Skybox/s1/back.png" );
  _faces.push_back( "../Skybox/s1/top.png" );
  _faces.push_back( "../Skybox/s1/bottom.png" );
  _faces.push_back( "../Skybox/s1/right.png" );
  _faces.push_back( "../Skybox/s1/left.png" );


  // Create ground VAO
  // -----------------
  if( _ground_VAO == 0 )
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

    GLfloat ground_vertices[] =
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
    glGenVertexArrays( 1, &_ground_VAO );
    glGenBuffers( 1, &_ground_VBO );
    glBindVertexArray( _ground_VAO );
    glBindBuffer( GL_ARRAY_BUFFER, _ground_VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( ground_vertices ), &ground_vertices, GL_STATIC_DRAW );
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
  if( _pipeline_type == FORWARD_RENDERING )
  {
    glGenFramebuffers( 1, &_window->_toolbox->_temp_hdr_FBO );
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_temp_hdr_FBO );
    glGenTextures( 2, _window->_toolbox->_temp_tex_color_buffer );

    if( _multi_sample )
    { 
      // Multi sample textures setting
      for( unsigned int i = 0; i < 2; i++ ) 
      {
        _window->_toolbox->SetFboMultiSampleTexture( _window->_toolbox->_temp_tex_color_buffer[ i ],
                                                     _nb_multi_sample,
                                                     GL_RGB16F,
                                                     _window->_width,
                                                     _window->_height,
                                                     GL_COLOR_ATTACHMENT0 + i );
      }
      unsigned int attachments2[ 2 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
      glDrawBuffers( 2, attachments2 );

      // Multi sample RBO link
      _window->_toolbox->LinkMultiSampleRbo( _window->_toolbox->_temp_depth_RBO,
                                             _nb_multi_sample, 
                                             _window->_width,
                                             _window->_height );
    }
    else
    { 
      // Textures setting
      for( unsigned int i = 0; i < 2; i++ ) 
      {
        _window->_toolbox->SetFboTexture( _window->_toolbox->_temp_tex_color_buffer[ i ],
                                          GL_RGB16F,
                                          _window->_width,
                                          _window->_height,
                                          GL_COLOR_ATTACHMENT0 + i );
      }
      unsigned int attachments2[ 2 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
      glDrawBuffers( 2, attachments2 );
      
      // RBO link
      _window->_toolbox->LinkRbo( _window->_toolbox->_temp_depth_RBO,
                                  _window->_width,
                                  _window->_height );
    }
    if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
      std::cout << "Framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );


    // Create final color buffer
    // -------------------------
    glGenFramebuffers( 1, &_window->_toolbox->_final_hdr_FBO );
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_final_hdr_FBO );
    glGenTextures( 2, _window->_toolbox->_final_tex_color_buffer );

    for( unsigned int i = 0; i < 2; i++ ) 
    {
      _window->_toolbox->SetFboTexture( _window->_toolbox->_final_tex_color_buffer[ i ],
                                        GL_RGB16F,
                                        _window->_width,
                                        _window->_height,
                                        GL_COLOR_ATTACHMENT0 + i );
    }

    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
    {
      std::cout << "Framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );
  }

  
  // Create pingpong buffer
  // ----------------------
  glGenFramebuffers( 1, &_window->_toolbox->_pingpong_FBO );
  glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_pingpong_FBO );
  glGenTextures( 2, _window->_toolbox->_pingpong_color_buffers );
  for( unsigned int i = 0; i < 2; i++ )
  {
    _window->_toolbox->SetFboTexture( _window->_toolbox->_pingpong_color_buffers[ i ],
                                      GL_RGB16F,
                                      _window->_width * _blur_downsample,
                                      _window->_height * _blur_downsample,
                                      GL_COLOR_ATTACHMENT0 + i );
  }
  if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
  {
    std::cout << "Framebuffer not complete!" << std::endl;
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
  
  PointLight::SetLightsMultiplier( 100.0 );

  for( int row = 0; row < 5; row++ )
  {
    for( int column = 0; column < 5; column++ )
    {
      _lights.push_back( PointLight( glm::vec3( -4.0 + ( row * 2.0 ), 1.0, -4.0 + ( column * 2.0 ) ),
                                     glm::vec3( 1.0, 1.0, 1.0 ),
                                     0.05,
                                     3.0 ) );    
    }
  }

  for( unsigned int i = 0; i < _lights.size(); i++ )
  {
    _lights[ i ]._intensity *= PointLight::GetLightsMultiplier();
  }
}

void Scene::ObjectsInitialization()
{
  glm::vec3 position;


  // Tables object initialization
  // ----------------------------
  for( int i = 0; i < 20; i++ )
  { 

    switch( i % 3 )
    {
      case 0:
        position = glm::vec3( -1.5, 2.5, 2.0 - ( i * 0.25 ) );
        break;
      
      case 1:
        position = glm::vec3( 0.0, 2.5, 2.0 - ( i * 0.25 ) );
        break;

      case 2:
        position = glm::vec3( 1.5, 2.5, 2.0 - ( i * 0.25 ) );
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
                               false,
                               0.75,
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
                         false,
                         0.75,
                         false,
                         1.0 );

}

void Scene::IBLCubeMapsInitialization()
{ 
  std::vector< std::string > hdr_texture_paths;
  hdr_texture_paths.push_back( std::string( "../Skybox/hdr skybox 2/Ridgecrest_Road_Ref.hdr" ) );
  hdr_texture_paths.push_back( std::string( "../Skybox/hdr skybox 1/Arches_E_PineTree_3k.hdr" ) );
  hdr_texture_paths.push_back( std::string( "../Skybox/hdr skybox 3/QueenMary_Chimney_Ref.hdr" ) );

  for( unsigned int cube_map_it = 0; cube_map_it < hdr_texture_paths.size(); cube_map_it++ )
  {
    unsigned int hdr_texture;
    unsigned int env_cubeMap;
    unsigned int irradiance_cubeMap;
    unsigned int pre_filter_cubeMap;


    // Load hdr skybox texture
    // -----------------------
    _window->_toolbox->_hdr_image_manager->stbi_set_flip_vertically_on_load( true );
    int width, height, nrComponents;
    
    float * data = _window->_toolbox->_hdr_image_manager->stbi_loadf( hdr_texture_paths[ cube_map_it ].c_str(), &width, &height, &nrComponents, 0 );
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
    // ---------------------------------------------------
    glGenFramebuffers( 1, &_window->_toolbox->_capture_FBO );
    glGenRenderbuffers( 1, &_window->_toolbox->_capture_RBO );
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_capture_FBO );
    glBindRenderbuffer( GL_RENDERBUFFER, _window->_toolbox->_capture_RBO );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _res_env_cubeMap, _res_env_cubeMap );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _window->_toolbox->_capture_RBO );


    // Gen cubemap textures
    // --------------------
    glGenTextures( 1, &env_cubeMap );
    glBindTexture( GL_TEXTURE_CUBE_MAP, env_cubeMap );
    for( unsigned int i = 0; i < 6; ++i )
    {
      glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
      0, 
      GL_RGB16F, 
      _res_env_cubeMap, 
      _res_env_cubeMap, 
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

    
    // Convert hdr texture into cube map texture
    // -----------------------------------------
    _cube_map_converter_shader.Use();
    glUniform1i(glGetUniformLocation( _cube_map_converter_shader._program, "uEquirectangularMap" ), 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, hdr_texture );
    glUniformMatrix4fv( glGetUniformLocation( _cube_map_converter_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_projection_matrix ) );

    glViewport( 0, 0, _res_env_cubeMap, _res_env_cubeMap ); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_capture_FBO );
    for( unsigned int i = 0; i < 6; ++i )
    {
      glUniformMatrix4fv( glGetUniformLocation( _cube_map_converter_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_view_matrices[ i ] ) );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, env_cubeMap, 0 );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      _window->_toolbox->RenderCube();
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    glBindTexture( GL_TEXTURE_CUBE_MAP, env_cubeMap );
    glGenerateMipmap( GL_TEXTURE_CUBE_MAP ); // generate mipmaps from first mip face (combatting visible dots artifact)

    
    // Gen irradiance cube map textures
    // --------------------------------
    glGenTextures( 1, &irradiance_cubeMap );
    glBindTexture( GL_TEXTURE_CUBE_MAP, irradiance_cubeMap );
    for( unsigned int i = 0; i < 6; ++i )
    {
      glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0,
                    GL_RGB16F,
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

    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_capture_FBO );
    glBindRenderbuffer( GL_RENDERBUFFER, _window->_toolbox->_capture_RBO );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _res_irradiance_cubeMap, _res_irradiance_cubeMap );


    // Diffuse irradiance cube map calculation  
    // ---------------------------------------
    _diffuse_irradiance_shader.Use();
    glUniformMatrix4fv( glGetUniformLocation( _diffuse_irradiance_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_projection_matrix ) );
    glUniform1f( glGetUniformLocation( _diffuse_irradiance_shader._program, "uSampleDelta" ), _irradiance_sample_delta );
    glUniform1i( glGetUniformLocation( _diffuse_irradiance_shader._program, "uEnvironmentMap" ), 0 );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, env_cubeMap );
    
    glViewport( 0, 0, _res_irradiance_cubeMap, _res_irradiance_cubeMap ); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_capture_FBO );
    for( unsigned int i = 0; i < 6; ++i )
    {
      glUniformMatrix4fv( glGetUniformLocation( _diffuse_irradiance_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_view_matrices[ i ] ) );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_cubeMap, 0 );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      _window->_toolbox->RenderCube();
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    
    // Create pre filter cube map textures
    // -----------------------------------
    glGenTextures( 1, &pre_filter_cubeMap );
    glBindTexture( GL_TEXTURE_CUBE_MAP, pre_filter_cubeMap );
    for( unsigned int i = 0; i < 6; ++i )
    {
      glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, 
                    GL_RGB16F, 
                    _res_pre_filter_cubeMap, 
                    _res_pre_filter_cubeMap, 
                    0, 
                    GL_RGB, 
                    GL_FLOAT, 
                    nullptr );
    }
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glGenerateMipmap( GL_TEXTURE_CUBE_MAP ); // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory 


    // Compute pre filter cube map
    // ---------------------------
    _specular_pre_filter_shader.Use();
    glUniform1i( glGetUniformLocation( _specular_pre_filter_shader._program, "uEnvironmentMap" ), 0 );
    glUniformMatrix4fv( glGetUniformLocation( _specular_pre_filter_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_projection_matrix ) );
    glUniform1f( glGetUniformLocation( _specular_pre_filter_shader._program, "uCubeMapRes" ), _res_pre_filter_cubeMap );
    glUniform1ui( glGetUniformLocation( _specular_pre_filter_shader._program, "uSampleCount" ), _pre_filter_sample_count );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, env_cubeMap );

    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_capture_FBO );

    unsigned int max_mip_Level = 5;
    for( unsigned int mip = 0; mip < max_mip_Level; mip++ )
    {
      // Reisze framebuffer according to mip-level size.
      unsigned int mip_width  = _res_pre_filter_cubeMap * std::pow( 0.5, mip );
      unsigned int mip_height = mip_width;
      glBindRenderbuffer( GL_RENDERBUFFER, _window->_toolbox->_capture_RBO );
      glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_width, mip_height );
      glViewport( 0, 0, mip_width, mip_height );

      // Set roughness level for wich we need to render
      float roughness = ( float )mip / ( float )( max_mip_Level - 1 );
      glUniform1f( glGetUniformLocation( _specular_pre_filter_shader._program, "uRoughness" ), roughness );

      // Compute pre filter cube map for a given mip level and roughness level
      for( unsigned int i = 0; i < 6; i++ )
      {
        glUniformMatrix4fv( glGetUniformLocation( _specular_pre_filter_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_view_matrices[ i ] ) );
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, pre_filter_cubeMap, mip );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        _window->_toolbox->RenderCube();
      }
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );


    // Save texture IDs
    // ----------------
    _hdr_textures.push_back( hdr_texture );
    _env_cubeMaps.push_back( env_cubeMap);
    _irradiance_cubeMaps.push_back( irradiance_cubeMap );
    _pre_filter_cubeMaps.push_back( pre_filter_cubeMap );
  }
}

void Scene::ShadersInitialization()
{

  // Compilation des shaders
  // -----------------------
  _forward_pbr_shader.SetShaderClassicPipeline(         "../Shaders/forward_pbr_lighting.vs",   "../Shaders/forward_pbr_lighting.fs" );
  _skybox_shader.SetShaderClassicPipeline(              "../Shaders/skybox.vs",                 "../Shaders/skybox.fs" );
  _flat_color_shader.SetShaderClassicPipeline(          "../Shaders/flat_color.vs",             "../Shaders/flat_color.fs" );
  _observer_shader.SetShaderClassicPipeline(            "../Shaders/observer.vs",               "../Shaders/observer.fs" );
  _blur_shader.SetShaderClassicPipeline(                "../Shaders/post_process.vs",           "../Shaders/blur.fs" );
  _post_process_shader.SetShaderClassicPipeline(        "../Shaders/post_process.vs",           "../Shaders/post_process.fs" );
  _MS_blit_shader.SetShaderClassicPipeline(             "../Shaders/multisample_blit.vs",       "../Shaders/multisample_blit.fs" );
  _cube_map_converter_shader.SetShaderClassicPipeline(  "../Shaders/cube_map_converter.vs",     "../Shaders/cube_map_converter.fs" );
  _diffuse_irradiance_shader.SetShaderClassicPipeline(  "../Shaders/cube_map_converter.vs",     "../Shaders/diffuse_irradiance.fs" );
  _geometry_pass_shader.SetShaderClassicPipeline(       "../Shaders/deferred_geometry_pass.vs", "../Shaders/deferred_geometry_pass.fs" );
  _lighting_pass_shader.SetShaderClassicPipeline(       "../Shaders/deferred_lighting_pass.vs", "../Shaders/deferred_lighting_pass.fs" );
  _empty_shader.SetShaderClassicPipeline(               "../Shaders/flat_color.vs",             "../Shaders/empty.fs" );
  _specular_pre_filter_shader.SetShaderClassicPipeline( "../Shaders/cube_map_converter.vs",     "../Shaders/specular_pre_filter.fs" );


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
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uGbufferPositionAndBloom" ), 0 ) ;
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uGbufferNormalAndBloomBrightness" ), 1 );
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uGbufferAlbedo" ), 2 ) ;
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uGbufferRougnessMetalnessAO" ), 3 );
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uIrradianceCubeMap" ), 4 ); 
  glUseProgram( 0 );

  _skybox_shader.Use();
  glUniform1i( glGetUniformLocation( _skybox_shader._program, "uSkyboxTexture" ), 0 );
  glUseProgram( 0 );

  _observer_shader.Use();
  glUniform1i( glGetUniformLocation( _observer_shader._program, "uTexture1" ), 0 );
  glUseProgram( 0 );

  _MS_blit_shader.Use();
  glUniform1i( glGetUniformLocation( _MS_blit_shader._program, "uTexture1" ), 0 );
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

  // Volume sphere model loading
  _sphere_model = new Model();
  _sphere_model->Load_Model( "../Models/volume_sphere/volume_sphere.obj", 2, "VolumeSphere" );
  _sphere_model->Print_info_model();  
}

void Scene::DeferredBuffersInitialization()
{

  // G buffer initialization
  // -----------------------
  glGenFramebuffers( 1, &_g_buffer_FBO );
  glBindFramebuffer( GL_FRAMEBUFFER, _g_buffer_FBO );

  unsigned int texture_id;

  // Position buffer && Bloom bool
  glGenTextures( 1, &texture_id );
  glBindTexture( GL_TEXTURE_2D, texture_id );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, _window->_width, _window->_height, 0, GL_RGBA, GL_FLOAT, NULL );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0 );
  _g_buffer_textures.push_back( texture_id );

  // Normal buffer && BloomBrightness
  glGenTextures( 1, &texture_id );
  glBindTexture( GL_TEXTURE_2D, texture_id );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, _window->_width, _window->_height, 0, GL_RGBA, GL_FLOAT, NULL );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture_id, 0 );
  _g_buffer_textures.push_back( texture_id );

  // Albedo buffer
  glGenTextures( 1, &texture_id );
  glBindTexture( GL_TEXTURE_2D, texture_id );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, _window->_width, _window->_height, 0, GL_RGB, GL_FLOAT, NULL );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, texture_id, 0 );
  _g_buffer_textures.push_back( texture_id );

  // Roughness && Metalness && AO
  glGenTextures( 1, &texture_id );
  glBindTexture( GL_TEXTURE_2D, texture_id );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, _window->_width, _window->_height, 0, GL_RGB, GL_FLOAT, NULL );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, texture_id, 0 );
  _g_buffer_textures.push_back( texture_id );

  // Depth
  glGenTextures( 1, &texture_id );
  glBindTexture( GL_TEXTURE_2D, texture_id );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, _window->_width, _window->_height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL );
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture_id, 0 );
  _g_buffer_textures.push_back( texture_id ); 

  // Lighting && brightest texture
  for( unsigned int i = 0; i < 2; i++ ) 
  {
    glGenTextures( 1, &texture_id );
    glBindTexture( GL_TEXTURE_2D, texture_id );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, _window->_width, _window->_height, 0, GL_RGB, GL_FLOAT, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );  
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4 + i, GL_TEXTURE_2D, texture_id, 0 );
    _g_buffer_textures.push_back( texture_id );
  }

  if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
  {
    std::cout << "ERROR : G-buffer's FBO not complete" << std::endl;
  }

  glBindTexture( GL_TEXTURE_2D, 0 );
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void Scene::SceneForwardRendering()
{

  // Matrices setting
  // ----------------
  glm::mat4 model_matrix;


  // Bind correct buffer for drawing
  // -------------------------------
  glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_temp_hdr_FBO );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


  // Draw skybox
  // -----------
  glDepthMask( GL_FALSE ); // desactivé juste pour draw la skybox
  _skybox_shader.Use();   
  glm::mat4 Skyboxview_matrix = glm::mat4( glm::mat3( _camera->_view_matrix ) );  // Remove any translation component of the view matrix

  model_matrix = glm::mat4( 1.0f );
  glUniformMatrix4fv( glGetUniformLocation( _skybox_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _skybox_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( Skyboxview_matrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _skybox_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
  glUniform1f( glGetUniformLocation( _skybox_shader._program, "uAlpha" ), 1.0 );

  glUniform1i( glGetUniformLocation( _skybox_shader._program, "uBloom" ), false );
  glUniform1f( glGetUniformLocation( _skybox_shader._program, "uBloomBrightness" ), _ground1->_bloom_brightness );

  glActiveTexture( GL_TEXTURE0 );
  //glBindTexture( GL_TEXTURE_CUBE_MAP, _irradiance_cubeMaps[ _current_env ] ); // bind les 6 textures du cube map 
  //glBindTexture( GL_TEXTURE_CUBE_MAP, _env_cubeMaps[ _current_env ] ); // bind les 6 textures du cube map 
  glBindTexture( GL_TEXTURE_CUBE_MAP, _pre_filter_cubeMaps[ _current_env ] ); // bind les 6 textures du cube map 

  _window->_toolbox->RenderCube();

  glDepthMask( GL_TRUE );  // réactivé pour draw le reste
  glUseProgram( 0 );


  // Draw lamps
  // ----------
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  _flat_color_shader.Use();

  for( int i = 0; i < _lights.size(); i++ )
  {
    model_matrix= glm::mat4();
    model_matrix = glm::translate( model_matrix, _lights[ i ]._position );
    model_matrix = glm::scale( model_matrix, glm::vec3( 0.04f ) ); 
    glm::vec3 lampColor = _lights[ i ]._color * _lights[ i ]._intensity;
    glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uViewMatrix" ) , 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniform3f( glGetUniformLocation( _flat_color_shader._program, "uColor" ), lampColor.x, lampColor.y, lampColor.z );

    glUniform1i( glGetUniformLocation( _flat_color_shader._program, "uBloom" ), true );
    glUniform1f( glGetUniformLocation( _flat_color_shader._program, "uBloomBrightness" ), 1.0f );

    _sphere_model->Draw( _flat_color_shader );
  }
  glBindVertexArray( 0 );
  glUseProgram( 0 );


  // Draw ground1
  // ------------
  _forward_pbr_shader.Use();

  model_matrix = glm::mat4();

  model_matrix = glm::translate( model_matrix, _ground1->_position );
  model_matrix = glm::rotate( model_matrix, _ground1->_angle, glm::vec3( -1.0, 0.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, _ground1->_scale ); 

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
  glBindTexture( GL_TEXTURE_CUBE_MAP, _irradiance_cubeMaps[ _current_env ] ); // bind les 6 textures du cube map 

  glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );

  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightCount" ), _lights.size() );

  for( int i = 0; i < _lights.size(); i++ )
  {
    string temp = to_string( i );
    glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, ( "uLightPos[" + temp + "]" ).c_str() ),1, &_lights[ i ]._position[ 0 ] );
    glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, ( "uLightColor[" + temp + "]" ).c_str() ),1, &_lights[ i ]._color[ 0 ] );
    glUniform1f(  glGetUniformLocation( _forward_pbr_shader._program, ( "uLightIntensity[" + temp + "]" ).c_str() ), _lights[ i ]._intensity );
  }

  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _ground1->_bloom );
  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _ground1->_bloom_brightness );

  glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewPos" ), 1, &_camera->_position[ 0 ] );

  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _ground1->_alpha );
  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _ground1->_id );    

  glBindVertexArray( _ground_VAO );
  glDrawArrays( GL_TRIANGLES, 0, 6 );
  glBindVertexArray( 0 );
  glUseProgram( 0 );
 

  // Draw tables
  // -----------
  _forward_pbr_shader.Use();

  for( int i = 0; i < _tables.size(); i++ )
  {
    model_matrix = glm::mat4();

    model_matrix = glm::translate( model_matrix, _tables[ i ]._position );
    model_matrix = glm::rotate( model_matrix, _tables[ i ]._angle, glm::vec3( -1.0, 0.0 , 0.0 ) );
    model_matrix = glm::scale( model_matrix, _tables[ i ]._scale ); 

    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _irradiance_cubeMaps[ _current_env ] ); 

    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );

    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightCount" ), _lights.size() );

    for( int i = 0; i < _lights.size(); i++ )
    {
      string temp = to_string( i );
      glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, ( "uLightPos[" + temp + "]" ).c_str() ),1, &_lights[ i ]._position[ 0 ] );
      glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, ( "uLightColor[" + temp + "]" ).c_str() ),1, &_lights[ i ]._color[ 0 ] );
      glUniform1f(  glGetUniformLocation( _forward_pbr_shader._program, ( "uLightIntensity[" + temp + "]" ).c_str() ), _lights[ i ]._intensity );
    }

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _tables[ i ]._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _tables[ i ]._bloom_brightness );

    glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewPos" ), 1, &_camera->_position[ 0 ] );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _tables[ i ]._alpha );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _tables[ i ]._id );    

    _table_model->Draw( _forward_pbr_shader );
  } 
  glUseProgram( 0 );

  glDisable( GL_CULL_FACE );

  // Unbinding    
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glBindVertexArray( 0 );


  // Blit multi sample texture to classic texture
  // --------------------------------------------
  if( _multi_sample )
  {
    // Bind and clear normal texture where we need to blit
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_final_hdr_FBO );
    unsigned int attachments2[ 2 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers( 2, attachments2 );
    glClear( GL_COLOR_BUFFER_BIT );

    // Convert multi sample texture into normal texture
    _MS_blit_shader.Use();

    glDrawBuffer( GL_COLOR_ATTACHMENT0 );
    
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, _window->_toolbox->_temp_tex_color_buffer[ 0 ] );
    glUniform1i( glGetUniformLocation( _MS_blit_shader._program, "uSampleCount" ), _nb_multi_sample );    
    _window->_toolbox->RenderQuad();

    // Same convert with brightness texture ( bloom )
    glDrawBuffer( GL_COLOR_ATTACHMENT1 );
    
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, _window->_toolbox->_temp_tex_color_buffer[ 1 ] );
    glUniform1i( glGetUniformLocation( _MS_blit_shader._program, "uSampleCount" ), _nb_multi_sample );
    _window->_toolbox->RenderQuad();

    glUseProgram( 0 );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  }
}

void Scene::DeferredGeometryPass( glm::mat4 * iProjectionMatrix,
                                  glm::mat4 * iViewMatrix )
{ 
  glm::mat4 model_matrix;


  // Bind and clear G-buffer textures
  // --------------------------------
  unsigned int attachments[ 4 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
  glDrawBuffers( 4, attachments );

  // Only the geometry pass updates the depth buffer
  glDepthMask( GL_TRUE );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // Use depth test while drawing G-buffer textures
  glEnable( GL_DEPTH_TEST );

  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );

  glDisable( GL_BLEND );


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

  glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( *iViewMatrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( *iProjectionMatrix ) );

  glUniform1f( glGetUniformLocation( _geometry_pass_shader._program, "uBloom" ), _ground1->_bloom );
  glUniform1f( glGetUniformLocation( _geometry_pass_shader._program, "uBloomBrightness" ), _ground1->_bloom_brightness );

  glBindVertexArray( _ground_VAO );
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

    glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( *iViewMatrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( *iProjectionMatrix ) );

    glUniform1f( glGetUniformLocation( _geometry_pass_shader._program, "uBloom" ), _tables[ i ]._bloom );
    glUniform1f( glGetUniformLocation( _geometry_pass_shader._program, "uBloomBrightness" ), _tables[ i ]._bloom_brightness );

    _table_model->Draw( _geometry_pass_shader );
  } 
  glBindVertexArray( 0 );
  glUseProgram( 0 );

  // Only the geometry pass modify the depth buffer, then disable after it
  glDepthMask( GL_FALSE );
}

void Scene::DeferredLightingPass( glm::mat4 * iProjectionMatrix,
                                  glm::mat4 * iViewMatrix )
{ 
  float screen_size[ 2 ] = { ( float )_window->_width, ( float )_window->_height };
  glm::mat4 model_matrix;

  // Enable stencil test for stencil pass and lighting pass
  glEnable( GL_STENCIL_TEST );

  for( int i = 0; i < _lights.size(); i++ )
  {
    
    // Stencil pass
    // ------------
    _empty_shader.Use();   

    // Don't draw anything during the stencil pass, juste modify the stencil buffer
    glDrawBuffer( GL_NONE );
    
    // Need depth test enable to perform stencil buffer modification     
    glEnable( GL_DEPTH_TEST );

    // Need both volume sphere faces to perform stencil buffer modification
    glDisable( GL_CULL_FACE );

    glClear( GL_STENCIL_BUFFER_BIT );

    // Set stencil operation correctly
    glStencilFunc( GL_ALWAYS, 0, 0 );
    glStencilOpSeparate( GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP );
    glStencilOpSeparate( GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP );

    // Set correct model matrix
    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, _lights[ i ]._position );
    model_matrix = glm::scale( model_matrix, glm::vec3( _lights[ i ]._max_lighting_distance ) ); 

    // Uniforms
    glUniformMatrix4fv( glGetUniformLocation( _empty_shader._program, "uViewMatrix" ) , 1, GL_FALSE, glm::value_ptr( *iViewMatrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _empty_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _empty_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( *iProjectionMatrix ) );

    _sphere_model->Draw( _empty_shader );
    glBindVertexArray( 0 );
    
    glUseProgram( 0 );


    // Lighting pass
    // -------------

    // Bind the rendered frames
    unsigned int attachments[ 2 ] = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
    glDrawBuffers( 2, attachments );
 
    _lighting_pass_shader.Use();   

    // Bind lighting input texture
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, _g_buffer_textures[ 0 ] );  
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, _g_buffer_textures[ 1 ] ); 
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, _g_buffer_textures[ 2 ] ); 
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture( GL_TEXTURE_2D, _g_buffer_textures[ 3 ] );
    glActiveTexture( GL_TEXTURE4 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _irradiance_cubeMaps[ _current_env ] );

    // Set stencil test to pass only for the stencil values calculated before, when not equal 0 
    glStencilFunc( GL_NOTEQUAL, 0, 0xFF );

    // Don't need depth test anymore at this point
    glDisable( GL_DEPTH_TEST );

    // Set additional blending for all point light result
    glEnable( GL_BLEND );
    glBlendEquation( GL_FUNC_ADD );
    glBlendFunc( GL_ONE, GL_ONE );

    // Front face culling to still calculate lighting when camera is inside the volume sphere
    glEnable( GL_CULL_FACE );
    glCullFace( GL_FRONT );

    glUniformMatrix4fv( glGetUniformLocation( _lighting_pass_shader._program, "uViewMatrix" ) , 1, GL_FALSE, glm::value_ptr( *iViewMatrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _lighting_pass_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _lighting_pass_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( *iProjectionMatrix ) );

    glUniform3fv( glGetUniformLocation( _lighting_pass_shader._program, "uViewPos" ), 1, &_camera->_position[ 0 ] );
    glUniform3fv( glGetUniformLocation( _lighting_pass_shader._program, "uLightPos" ), 1, &_lights[ i ]._position[ 0 ] );
    glUniform3fv( glGetUniformLocation( _lighting_pass_shader._program, "uLightColor" ), 1, &_lights[ i ]._color[ 0 ] );
    glUniform1f(  glGetUniformLocation( _lighting_pass_shader._program, "uLightIntensity" ), _lights[ i ]._intensity );
    glUniform1f(  glGetUniformLocation( _lighting_pass_shader._program, "uLightMaxDistance" ), _lights[ i ]._max_lighting_distance );
    glUniform2fv( glGetUniformLocation( _lighting_pass_shader._program, "uScreenSize" ), 1, screen_size );

    _sphere_model->Draw( _lighting_pass_shader );
    glBindVertexArray( 0 );

    glUseProgram( 0 );

    glDisable( GL_BLEND );
  }
  // Disable stencil test
  glDisable( GL_STENCIL_TEST );

  glCullFace( GL_BACK );
  glDisable( GL_BLEND );
}

void Scene::SceneDeferredRendering()
{

  // Projection and view matrices setting
  glm::mat4 model_matrix;

  // Bind and clear the rendered frames
  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, _g_buffer_FBO );
  unsigned int attachments[ 2 ] = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
  glDrawBuffers( 2, attachments );
  glClear( GL_COLOR_BUFFER_BIT );


  // Deferred rendering G-buffer pass
  // --------------------------------
  DeferredGeometryPass( &_camera->_projection_matrix,
                        &_camera->_view_matrix );


  // Deferred rendering lighting pass
  // --------------------------------
  DeferredLightingPass( &_camera->_projection_matrix,
                        &_camera->_view_matrix );


  // Forward render lamps sphere
  // ---------------------------
  glEnable( GL_DEPTH_TEST );
  glDepthMask( GL_FALSE );

  _flat_color_shader.Use();
  for( int i = 0; i < _lights.size(); i++ )
  {
    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, _lights[ i ]._position );
    model_matrix = glm::scale( model_matrix, glm::vec3( 0.04f ) ); 
    glm::vec3 lamp_color = _lights[ i ]._color * _lights[ i ]._intensity;
    glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uViewMatrix" ) , 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniform3f( glGetUniformLocation( _flat_color_shader._program, "uColor" ), lamp_color.x, lamp_color.y, lamp_color.z );

    glUniform1i( glGetUniformLocation( _flat_color_shader._program, "uBloom" ), true );
    glUniform1f( glGetUniformLocation( _flat_color_shader._program, "uBloomBrightness" ), 1.0f );

    _sphere_model->Draw( _flat_color_shader );
  }
  glUseProgram( 0 );
  

  // Forward render lights volume
  // ----------------------------
  if( _render_lights_volume )
  {
    _flat_color_shader.Use();
    for( int i = 0; i < _lights.size(); i++ )
    {
      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, _lights[ i ]._position );
      model_matrix = glm::scale( model_matrix, glm::vec3( _lights[ i ]._max_lighting_distance ) ); 
      glm::vec3 sphere_color = glm::vec3( 0.0, 0.0, 1.0 ) * _lights[ i ]._intensity;
      glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uViewMatrix" ) , 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
      glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
      glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
      glUniform3f( glGetUniformLocation( _flat_color_shader._program, "uColor" ), sphere_color.x, sphere_color.y, sphere_color.z );

      glUniform1i( glGetUniformLocation( _flat_color_shader._program, "uBloom" ), false );

      _sphere_model->Draw( _flat_color_shader );
    }
    glUseProgram( 0 );
  }

  // Unbind current FBO
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void Scene::BlurProcess()
{ 
  bool first_ite = true;
  int horizontal = 1; 

  glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_pingpong_FBO ); 
  _blur_shader.Use();
  
  for( unsigned int i = 0; i < _blur_pass_count; i++ )
  {
    horizontal == 0 ? glDrawBuffer( GL_COLOR_ATTACHMENT0 ) : glDrawBuffer( GL_COLOR_ATTACHMENT1 ); 
    glClear( GL_COLOR_BUFFER_BIT );

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
        if( _pipeline_type == FORWARD_RENDERING )
        {
          glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_temp_tex_color_buffer[ 1 ] ); 
        }
        else
        {
          glBindTexture( GL_TEXTURE_2D, _g_buffer_textures[ 6 ] ); 
        }
      }
    }

    glUniform1f( glGetUniformLocation( _blur_shader._program, "uHorizontal" ), horizontal );
    glUniform1f( glGetUniformLocation( _blur_shader._program, "uOffsetFactor" ), _blur_offset_factor );
    _window->_toolbox->RenderQuad();
  }

  glUseProgram( 0 );
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );     
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
    if( _pipeline_type == FORWARD_RENDERING )
    {
      glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_temp_tex_color_buffer[ 0 ] );
    }
    else
    {
      glBindTexture( GL_TEXTURE_2D, _g_buffer_textures[ 5 ] );
    }
  }

  if( _bloom )
  {
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_pingpong_color_buffers[ 0 ] );
  }

  glUniform1i( glGetUniformLocation( _post_process_shader._program, "uBloom" ), _bloom );
  glUniform1f( glGetUniformLocation( _post_process_shader._program, "uExposure" ), _exposure );
  _window->_toolbox->RenderQuad();

  glUseProgram( 0 );
}