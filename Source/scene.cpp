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
  
  // near far
  _near = 0.01;
  _far  = 30.0;
  _shadow_near = 0.01;
  _shadow_far  = 30.0;

  // Frame exposure
  _exposure           = 1.0;

  // Init bloom parameters
  _bloom              = true;
  _blur_downsample    = 1.0;
  _blur_pass_count    = 6;
  _blur_offset_factor = 1.0;

  // Init multi sample parameters
  _multi_sample    = false;
  _nb_multi_sample = 4;

  // Init IBL parameters
  _current_env             = 2;
  _res_env_cubeMap         = 512;

  _res_irradiance_cubeMap  = 32;
  _irradiance_sample_delta = 0.025;
  
  _res_pre_filter_cubeMap   = 256;
  _pre_filter_sample_count  = 1024 * 1;
  _pre_filter_max_mip_Level = 5;

  _res_pre_brdf_texture  = 512;
  _pre_brdf_sample_count = 1024 * 1;    

  // Init tessellation parameters
  _tess_patch_vertices_count = 3;

  // Init omnidirectional shadow mapping parameters
  _depth_cubemap_res = 2048;

  // Lights volume
  _render_lights_volume = false;

  // Revolving door
  _door_angle = 0.0;
  _door_open  = false;


  // Scene data initialization
  // -------------------------

  _ground1_VAO = 0;
  _ground1_VBO = 0;
  _ground1_IBO = 0;

  _wall1_VAO = 0;
  _wall1_VBO = 0;
  _wall1_IBO = 0;

  // Get pointer on the scene window
  _window = iParentWindow;
  
  // Create and init scene clock
  _clock = new Clock();

   // Create and init scene camera
  _camera = new Camera( glm::vec3( -3, 1.0, -3 ),
                        glm::vec3( 0.724498, -0.409127, 0.554724 ),
                        glm::vec3( 0.0f, 1.0f,  0.0f ),
                        37.44,
                        -24.0,
                        _near,
                        _far,
                        45.0f,
                        ( float )_window->_width,
                        ( float )_window->_height,
                        6.0 );

  // Create and init all shaders
  ShadersInitialization();

  // Create lights
  LightsInitialization(); 

  // Create all scene's objects
  ObjectsInitialization();

  // Load all scene models
  ModelsLoading(); 

  // Init scene data 
  SceneDataInitialization();

  // Init all IBL texture
  IBLInitialization();

  // Init tesselation parameters
  TesselationInitialization();

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
  if( _tex_albedo_ground1 )
    glDeleteTextures( 1, &_tex_albedo_ground1 );
  if( _tex_normal_ground1 )
    glDeleteTextures( 1, &_tex_normal_ground1 );
  if( _tex_height_ground1 )
    glDeleteTextures( 1, &_tex_height_ground1 );
  if( _tex_AO_ground1 )
    glDeleteTextures( 1, &_tex_AO_ground1 );
  if( _tex_roughness_ground1 )
    glDeleteTextures( 1, &_tex_roughness_ground1 );
  if( _tex_metalness_ground1 )
    glDeleteTextures( 1, &_tex_metalness_ground1 );


  // Delete VAOs
  // -----------
  if( _ground1_VAO )
    glDeleteVertexArrays( 1, &_ground1_VAO );
  

  // Delete VBOs
  // -----------
  if( _ground1_VBO )
    glDeleteBuffers( 1, &_ground1_VBO );


  // Delete FBOs
  // -----------
  if( _window->_toolbox->_temp_hdr_FBO )
    glDeleteFramebuffers( 1, &_window->_toolbox->_temp_hdr_FBO );
  if( _window->_toolbox->_final_hdr_FBO )
    glDeleteFramebuffers( 1, &_window->_toolbox->_final_hdr_FBO );
  

  // Delete RBOs
  // -----------
  if( _window->_toolbox->_temp_depth_RBO )
    glDeleteRenderbuffers( 1, &_window->_toolbox->_temp_depth_RBO );
}

void Scene::SceneDataInitialization()
{

  float anisotropy_value = 0.0f; 
  glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy_value );

  SDL_Surface * sdl_image_data = NULL;


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


  // Create ground type 1 VAO
  // ------------------------
  _window->_toolbox->CreatePlaneVAO( &_ground1_VAO,
                                     &_ground1_VBO,
                                     &_ground1_IBO,
                                     &_ground1_indices,
                                     40,
                                     _grounds_type1[ 0 ]._uv_scale.x );


  // Create wall type 1 VAO
  // ----------------------
  _window->_toolbox->CreatePlaneVAO( &_wall1_VAO,
                                     &_wall1_VBO,
                                     &_wall1_IBO,
                                     &_wall1_indices,
                                     40,
                                     _walls_type1[ 0 ]._uv_scale.x );


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

  
  // Create pingpong buffer and textures
  // -----------------------------------
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


  // Create depth cube map texture & FBO
  // -----------------------------------
  glGenFramebuffers( 1, &_window->_toolbox->_depth_map_FBO );

  glGenTextures( 1, &_window->_toolbox->_depth_cubemap );
  glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );
  for( unsigned int i = 0; i < 6; i++ )
  {
    glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                  0,
                  GL_DEPTH_COMPONENT,
                  _depth_cubemap_res,
                  _depth_cubemap_res,
                  0,
                  GL_DEPTH_COMPONENT,
                  GL_FLOAT,
                  NULL );
  }
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
  
  // Attach depth texture as FBO's depth buffer
  glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_depth_map_FBO );
  glFramebufferTexture( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _window->_toolbox->_depth_cubemap, 0 );
  glDrawBuffer( GL_NONE );
  glReadBuffer( GL_NONE );
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glBindTexture( GL_TEXTURE_2D, 0 );


  // Load ground1 albedo texture
  // --------------------------
  if( ( sdl_image_data = IMG_Load( "../Textures/ground1/albedo.png" ) ) != NULL )
  {
    _tex_albedo_ground1 = _window->_toolbox->CreateTextureFromData( sdl_image_data,
                                                                   GL_RGB,
                                                                   GL_RGB,
                                                                   true,
                                                                   true,
                                                                   anisotropy_value );
    SDL_FreeSurface( sdl_image_data );
  }
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
  }

  // Load ground1 normal texture
  // --------------------------
  if( ( sdl_image_data = IMG_Load( "../Textures/ground1/normal.png" ) ) != NULL )
  {
    _tex_normal_ground1 = _window->_toolbox->CreateTextureFromData( sdl_image_data,
                                                                   GL_RGB,
                                                                   GL_RGB,
                                                                   true,
                                                                   true,
                                                                   anisotropy_value );
    SDL_FreeSurface( sdl_image_data );
  }
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
  }


  // Load ground1 height texture
  // --------------------------
  if( ( sdl_image_data = IMG_Load( "../Textures/ground1/height.png" ) ) != NULL )
  {
    _tex_height_ground1 = _window->_toolbox->CreateTextureFromData( sdl_image_data,
                                                                   GL_R8,
                                                                   GL_RED,
                                                                   true,
                                                                   true,
                                                                   anisotropy_value );
    SDL_FreeSurface( sdl_image_data );
  }
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
  }


  // Load ground1 AO texture
  // ----------------------
  if( ( sdl_image_data = IMG_Load( "../Textures/ground1/AO.png" ) ) != NULL )
  {
    _tex_AO_ground1 = _window->_toolbox->CreateTextureFromData( sdl_image_data,
                                                               GL_R8,
                                                               GL_RED,
                                                               true,
                                                               true,
                                                               anisotropy_value );
    SDL_FreeSurface( sdl_image_data );
  }
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
  }


  // Load ground1 roughness texture 
  // -----------------------------
  if( ( sdl_image_data = IMG_Load( "../Textures/ground1/roughness.png" ) ) != NULL )
  {
    _tex_roughness_ground1 = _window->_toolbox->CreateTextureFromData( sdl_image_data,
                                                                      GL_R8,
                                                                      GL_RED,
                                                                      true,
                                                                      true,
                                                                      anisotropy_value );
    SDL_FreeSurface( sdl_image_data );
  }
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
  }


  // Load ground1 metalness texture  
  // -----------------------------
  if( ( sdl_image_data = IMG_Load( "../Textures/ground1/metalness.png" ) ) != NULL )
  {
    _tex_metalness_ground1 = _window->_toolbox->CreateTextureFromData( sdl_image_data,
                                                                      GL_R8,
                                                                      GL_RED,
                                                                      true,
                                                                      true,
                                                                      anisotropy_value );
    SDL_FreeSurface( sdl_image_data );
  }
  else
  {
    fprintf( stderr, "Erreur lors du chargement de la texture\n" );
  }
}

void Scene::LightsInitialization()
{ 
  _lights.clear();
  
  PointLight::SetLightsMultiplier( 30.0 );

  _lights.push_back( PointLight( glm::vec3( -2.0, 2.0, 0.0 ),
                                 glm::vec3( 1.0, 1.0, 1.0 ),
                                 0.5,
                                 3.0 ) );    

  for( unsigned int i = 0; i < _lights.size(); i++ )
  {
    _lights[ i ]._intensity *= PointLight::GetLightsMultiplier();
  }
}

void Scene::ObjectsInitialization()
{
  glm::vec3 position;
  glm::mat4 model_matrix = glm::mat4(); 

  _ground_size = 8.0;
  _wall_size   = _ground_size / 3.0;


  // _grounds type 1 object initialization ( room 1 ground & roof )
  // --------------------------------------------------------------
  for( int i = 0; i < 2; i ++ )
  {
    position = glm::vec3( -(_ground_size * 0.5 ) + 0.0, 0.0, -(_ground_size * 0.5 ) + 0.0 );
    position += glm::vec3( _ground_size * i, i * _wall_size, 0.0 );

    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, position );
    model_matrix = glm::rotate( model_matrix, ( float )_PI * ( float )i, glm::vec3( 0.0, 0.0 , 1.0 ) );
    glm::vec3 scale( _ground_size, 1.0, _ground_size );
    model_matrix = glm::scale( model_matrix, glm::vec3( _ground_size, 1.0, _ground_size ) ); 

    Object temp_object( 1,              // ID
                        model_matrix,   // model matrix
                        position,       // position
                        _PI_2,          // angle
                        scale,          // scale
                        glm::vec2( 6.0, 6.0 ),                          // uv scale
                        1.0,            // alpha
                        false,          // generate shadow
                        true,           // receiv shadow
                        1.0,            // shadow darkness
                        0.035,          // shadow bias
                        false,          // bloom
                        0.99,           // bloom bright value
                        false,          // opacity map
                        true,           // normal map
                        false,           // height map
                        0.12,           // displacement factor
                        0.15 );         // tessellation factor

    _grounds_type1.push_back( temp_object );
  }


  // _ink_bottle object initialization
  // ---------------------------------
  position = glm::vec3( 1.0, 0.6, 1.0 );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2 * -1.5f, glm::vec3( 0.0, 1.0 , 0.0 ) );
  glm::vec3 scale( 0.03, 0.03, 0.03 );
  model_matrix = glm::scale( model_matrix, scale );

  _ink_bottle.Set( Object( 2, // ID
                           model_matrix,
                           glm::vec3( 1.0, 0.6, 1.0 ),
                           0.0,
                           scale,
                           glm::vec2( 1.0, 1.0 ),
                           1.0,
                           true,
                           true,
                           1.0,
                           0.035,
                           false,
                           0.99,
                           true,
                           true,
                           false,
                           0.0,
                           0.0 ) );


  // _walls type 1 object initialization ( room 1 walls )
  // ----------------------------------------------------
  for( int i = 0; i < 12; i ++ )
  {
    if( i >= 0 && i < 3 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( 0.0, _wall_size, 0.0 );
      position -= glm::vec3( _wall_size - ( _wall_size * i ), 0.0, _wall_size );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0 , 0.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    if( i > 2 && i < 6 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( _wall_size - ( _wall_size * ( i - 3 ) ), 0.0, _wall_size * 2.0 );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( -1.0, 0.0 , 0.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    if( i > 5 && i < 9 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( _wall_size * 2.0, 0.0, _wall_size - ( _wall_size * ( i - 6 ) ) );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , 1.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    if( i > 8 && i < 12 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( -_wall_size, _wall_size, _wall_size - ( _wall_size * ( i - 9 ) ) );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , -1.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    Object temp_object( 4, // ID
                        model_matrix,
                        position,
                        _PI_2,
                        glm::vec3( _wall_size, 1.0, _wall_size ),
                        glm::vec2( 6.0 / 3.0, 6.0 / 3.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        false,
                        0.99,
                        false,
                        true,
                        false,
                        0.12,
                        0.15 / 3.0 );

    if( i != 1 && i != 10 )
    {
      _walls_type1.push_back( temp_object );
    }
  }


  // _walls type 2 object initialization ( entrance corridor ground & roof )
  // -----------------------------------------------------------------------
  for( int i = 0; i < 6; i ++ )
  {
    glm::vec3 scale( _wall_size, 1.0, _wall_size );

    if( i == 0 || i == 1 || i == 2 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( -( _wall_size * ( 2.0 + i ) ), 0.0, 0.0 );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::scale( model_matrix, scale );
    }

    if( i == 3 || i == 4 || i == 5 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( -_wall_size, _wall_size, 0.0 );

      if( i == 4 )
      {
        position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
        position += glm::vec3( -_wall_size * 2, _wall_size, 0.0 );
      }

      if( i == 5 )
      {
        position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
        position += glm::vec3( -_wall_size * 3, _wall_size, 0.0 );
      }

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI, glm::vec3( 0.0, 0.0 , 1.0 ) );
      model_matrix = glm::scale( model_matrix, scale );
    }

    Object temp_object( 5, // ID
                        model_matrix,
                        position,
                        _PI_2,
                        scale,
                        glm::vec2( 6.0 / 3.0, 6.0 / 3.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        false,
                        0.99,
                        false,
                        true,
                        false,
                        0.12,
                        0.15 / 3.0 );

    _walls_type1.push_back( temp_object );
  }


  // _walls type 3 object initialization ( entrance corridor walls )
  // ---------------------------------------------------------------
  for( int i = 0; i < 7; i ++ )
  { 
    glm::vec3 scale( _wall_size, 1.0, _wall_size );

    if( i == 0 || i == 1 || i == 2 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( -_wall_size * ( 2.0 + i ), _wall_size, 0.0 );
      
      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0 , 0.0 ) );
      model_matrix = glm::scale( model_matrix, scale );
    }

    if( i == 3 || i == 4 || i == 5 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( -_wall_size * ( 2.0 + ( i - 3 ) ), 0.0, _wall_size );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( -1.0, 0.0 , 0.0 ) );
      model_matrix = glm::scale( model_matrix, scale );
    }

    if( i == 6 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( -_wall_size * 4.0, _wall_size, 0.0 );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , -1.0 ) );
      model_matrix = glm::scale( model_matrix, scale );
    }

    Object temp_object( 6, // ID
                        model_matrix,
                        position,
                        _PI_2,
                        scale,
                        glm::vec2( 6.0 / 3.0, 6.0 / 3.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        false,
                        0.99,
                        false,
                        true,
                        false,
                        0.12,
                        0.15 / 3.0 );

    _walls_type1.push_back( temp_object );
  }


  // _revolving_door object initialization
  // -------------------------------------
  for( int i = 0; i < 3; i ++ )
  {   
    scale = glm::vec3( 0.105, 0.105, 0.105 );
    
    if( i == 0 )
    {
      position = glm::vec3( -4.8, 0.03, 0.0 );
    }

    if( i == 1 )
    {
      position = glm::vec3( 0.0, 0.03, -13.8 );
    }

    if( i == 2 )
    {
      position = glm::vec3( 13.8, 0.03, -_ground_size - ( _wall_size * 4 ) );
    }

    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, position );
    model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( -1.0, 0.0, 0.0 ) );
    
    if( i != 1 )
    {
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0, 1.0 ) );
    }

    model_matrix = glm::scale( model_matrix, scale );

    _revolving_door.push_back( Object( 7, // ID
                                       model_matrix,
                                       position,
                                       0.0,
                                       scale,
                                       glm::vec2( 1.0, 1.0 ),
                                       1.0,
                                       true,
                                       true,
                                       1.0,
                                       0.015,
                                       true,
                                       0.99,
                                       true,
                                       true,
                                       false,
                                       0.0,
                                       0.0 ) );
  }


  // _walls type 4 object initialization ( corridor room 1 to room 2 ground )
  // ------------------------------------------------------------------------
  for( int i = 0; i < 4; i ++ )
  { 
    glm::vec3 scale( _wall_size, 1.0, _wall_size );

    position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
    position += glm::vec3( 0.0, 0.0, -_wall_size * ( 2 + i ) );
    
    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, position );
    //model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0 , 0.0 ) );
    model_matrix = glm::scale( model_matrix, scale );

    Object temp_object( 8, // ID
                        model_matrix,
                        position,
                        _PI_2,
                        scale,
                        glm::vec2( 6.0 / 3.0, 6.0 / 3.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        false,
                        0.99,
                        false,
                        true,
                        false,
                        0.12,
                        0.15 / 3.0 );

    _walls_type1.push_back( temp_object );
  }


  // _walls type 5 object initialization ( corridor room 1 to room 2 roof )
  // ----------------------------------------------------------------------
  for( int i = 0; i < 4; i ++ )
  { 
    glm::vec3 scale( _wall_size, 1.0, _wall_size );

    position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
    position += glm::vec3( 0.0, _wall_size, -_wall_size * ( i + 1 ) );
    
    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, position );
    model_matrix = glm::rotate( model_matrix, ( float )_PI, glm::vec3( 1.0, 0.0 , 0.0 ) );
    model_matrix = glm::scale( model_matrix, scale );

    Object temp_object( 9, // ID
                        model_matrix,
                        position,
                        _PI_2,
                        scale,
                        glm::vec2( 6.0 / 3.0, 6.0 / 3.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        false,
                        0.99,
                        false,
                        true,
                        false,
                        0.12,
                        0.15 / 3.0 );

    _walls_type1.push_back( temp_object );
  }


  // _walls type 6 object initialization ( corridor room 1 to room 2 walls )
  // -----------------------------------------------------------------------
  for( int i = 0; i < 8; i ++ )
  { 
    glm::vec3 scale( _wall_size, 1.0, _wall_size );

    if( i < 4 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( _wall_size, 0.0, -_wall_size * ( 2 + i ) );
      
      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , 1.0 ) );
      model_matrix = glm::scale( model_matrix, scale );
    }

    if( i > 3 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( 0.0, _wall_size, -_wall_size * ( 2 + ( i - 4 ) ) );
      
      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , -1.0 ) );
      model_matrix = glm::scale( model_matrix, scale );
    }

    Object temp_object( 10, // ID
                        model_matrix,
                        position,
                        _PI_2,
                        scale,
                        glm::vec2( 6.0 / 3.0, 6.0 / 3.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        false,
                        0.99,
                        false,
                        true,
                        false,
                        0.12,
                        0.15 / 3.0 );

    _walls_type1.push_back( temp_object );
  }


  // _grounds type 2 object initialization ( room 2 ground )
  // ------------------------------------------------------
  scale = glm::vec3( _ground_size, 1.0, _ground_size );

  position = glm::vec3( -(_ground_size * 0.5 ) + 0.0, 0.0, -(_ground_size * 0.5 ) + 0.0 );
  position += glm::vec3( 0.0, 0.0, -_ground_size - ( _wall_size * 4 ) );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::scale( model_matrix, glm::vec3( _ground_size, 1.0, _ground_size ) ); 

  Object temp_object( 11,              // ID
                      model_matrix,   // model matrix
                      position,       // position
                      _PI_2,          // angle
                      scale,          // scale
                      glm::vec2( 6.0, 6.0 ),                          // uv scale
                      1.0,            // alpha
                      false,          // generate shadow
                      true,           // receiv shadow
                      1.0,            // shadow darkness
                      0.035,          // shadow bias
                      false,          // bloom
                      0.99,           // bloom bright value
                      false,          // opacity map
                      true,           // normal map
                      false,           // height map
                      0.12,           // displacement factor
                      0.15 );         // tessellation factor

  _grounds_type1.push_back( temp_object );


  // _grounds type 3 object initialization ( room 2 roof )
  // ------------------------------------------------------
  scale = glm::vec3( _ground_size, 1.0, _ground_size );

  position = glm::vec3( -(_ground_size * 0.5 ) + 0.0, 0.0, -(_ground_size * 0.5 ) + 0.0 );
  position += glm::vec3( 0.0, _wall_size, -_wall_size * 4 );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI, glm::vec3( 1.0, 0.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, glm::vec3( _ground_size, 1.0, _ground_size ) );

  temp_object = Object( 12,              // ID
                        model_matrix,   // model matrix
                        position,       // position
                        _PI_2,          // angle
                        scale,          // scale
                        glm::vec2( 6.0, 6.0 ),                          // uv scale
                        1.0,            // alpha
                        false,          // generate shadow
                        true,           // receiv shadow
                        1.0,            // shadow darkness
                        0.035,          // shadow bias
                        false,          // bloom
                        0.99,           // bloom bright value
                        false,          // opacity map
                        true,           // normal map
                        false,           // height map
                        0.12,           // displacement factor
                        0.15 );         // tessellation factor

  _grounds_type1.push_back( temp_object );


  // _walls type 7 object initialization ( room 2 walls )
  // ----------------------------------------------------
  glm::vec3 room2_offset_position( 0.0, 0.0, -_ground_size - (_wall_size * 4 ) );
  for( int i = 0; i < 12; i ++ )
  {
    if( i >= 0 && i < 3 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( 0.0, _wall_size, 0.0 );
      position -= glm::vec3( _wall_size - ( _wall_size * i ), 0.0, _wall_size );
      position += room2_offset_position;

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0 , 0.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    if( i > 2 && i < 6 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( _wall_size - ( _wall_size * ( i - 3 ) ), 0.0, _wall_size * 2.0 );
      position += room2_offset_position;

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( -1.0, 0.0 , 0.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    if( i > 5 && i < 9 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( _wall_size * 2.0, 0.0, _wall_size - ( _wall_size * ( i - 6 ) ) );
      position += room2_offset_position;

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , 1.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    if( i > 8 && i < 12 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( -_wall_size, _wall_size, _wall_size - ( _wall_size * ( i - 9 ) ) );
      position += room2_offset_position;

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , -1.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    Object temp_object( 13, // ID
                        model_matrix,
                        position,
                        _PI_2,
                        glm::vec3( _wall_size, 1.0, _wall_size ),
                        glm::vec2( 6.0 / 3.0, 6.0 / 3.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        false,
                        0.99,
                        false,
                        true,
                        false,
                        0.12,
                        0.15 / 3.0 );

    if( i != 4 && i != 7 )
    {
      _walls_type1.push_back( temp_object );
    }
  }


  // _walls type 8 object initialization ( corridor room 2 to room 3 ground )
  // ------------------------------------------------------------------------
  for( int i = 0; i < 4; i ++ )
  { 
    glm::vec3 scale( _wall_size, 1.0, _wall_size );

    position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
    position += room2_offset_position;
    position += glm::vec3( _wall_size * ( 2 + i ), 0.0, 0.0 );
        
    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, position );
    //model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0 , 0.0 ) );
    model_matrix = glm::scale( model_matrix, scale );

    Object temp_object( 14, // ID
                        model_matrix,
                        position,
                        _PI_2,
                        scale,
                        glm::vec2( 6.0 / 3.0, 6.0 / 3.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        false,
                        0.99,
                        false,
                        true,
                        false,
                        0.12,
                        0.15 / 3.0 );

    _walls_type1.push_back( temp_object );
  }


  // _walls type 9 object initialization ( corridor room 2 to room 3 roof )
  // ----------------------------------------------------------------------
  for( int i = 0; i < 4; i ++ )
  { 
    glm::vec3 scale( _wall_size, 1.0, _wall_size );

    position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
    position += room2_offset_position;
    position += glm::vec3( _wall_size * ( 3 + i ), _wall_size, 0.0 );
        
    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, position );
    model_matrix = glm::rotate( model_matrix, ( float )_PI, glm::vec3( 0.0, 0.0 , 1.0 ) );
    model_matrix = glm::scale( model_matrix, scale );

    Object temp_object( 15, // ID
                        model_matrix,
                        position,
                        _PI_2,
                        scale,
                        glm::vec2( 6.0 / 3.0, 6.0 / 3.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        false,
                        0.99,
                        false,
                        true,
                        false,
                        0.12,
                        0.15 / 3.0 );

    _walls_type1.push_back( temp_object );
  }


  // _walls type 10 object initialization ( corridor room 2 to room 3 walls )
  // ------------------------------------------------------------------------
  for( int i = 0; i < 8; i ++ )
  { 
    glm::vec3 scale( _wall_size, 1.0, _wall_size );

    if( i < 4 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += room2_offset_position;
      position += glm::vec3( _wall_size * ( 2 + i ), _wall_size, 0.0 );
      
      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0 , 0.0 ) );
      model_matrix = glm::scale( model_matrix, scale );
    }

    if( i > 3 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += room2_offset_position;
      position += glm::vec3( _wall_size * ( 2 + ( i - 4 ) ), 0.0, _wall_size );
      
      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( -1.0, 0.0 , 0.0 ) );
      model_matrix = glm::scale( model_matrix, scale );
    }

    Object temp_object( 16, // ID
                        model_matrix,
                        position,
                        _PI_2,
                        scale,
                        glm::vec2( 6.0 / 3.0, 6.0 / 3.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        false,
                        0.99,
                        false,
                        true,
                        false,
                        0.12,
                        0.15 / 3.0 );

    _walls_type1.push_back( temp_object );
  }


  // _grounds type 4 object initialization ( room 3 ground )
  // -------------------------------------------------------
  glm::vec3 room3_offset_position = room2_offset_position + glm::vec3( _ground_size + ( _wall_size * 4 ), 0.0, 0.0 );

  scale = glm::vec3( _ground_size, 1.0, _ground_size );

  position = glm::vec3( -(_ground_size * 0.5 ) + 0.0, 0.0, -(_ground_size * 0.5 ) + 0.0 );
  position += room3_offset_position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::scale( model_matrix, glm::vec3( _ground_size, 1.0, _ground_size ) ); 

  temp_object = Object( 17,              // ID
                        model_matrix,   // model matrix
                        position,       // position
                        _PI_2,          // angle
                        scale,          // scale
                        glm::vec2( 6.0, 6.0 ),                          // uv scale
                        1.0,            // alpha
                        false,          // generate shadow
                        true,           // receiv shadow
                        1.0,            // shadow darkness
                        0.035,          // shadow bias
                        false,          // bloom
                        0.99,           // bloom bright value
                        false,          // opacity map
                        true,           // normal map
                        false,           // height map
                        0.12,           // displacement factor
                        0.15 );         // tessellation factor

  _grounds_type1.push_back( temp_object );


  // _grounds type 5 object initialization ( room 3 roof )
  // -----------------------------------------------------
  scale = glm::vec3( _ground_size, 1.0, _ground_size );

  position = glm::vec3( -(_ground_size * 0.5 ) + 0.0, 0.0, -(_ground_size * 0.5 ) + 0.0 );
  position += room3_offset_position;
  position += glm::vec3( 0.0, _wall_size, _ground_size );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI, glm::vec3( 1.0, 0.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, glm::vec3( _ground_size, 1.0, _ground_size ) ); 

  temp_object = Object( 18,              // ID
                        model_matrix,   // model matrix
                        position,       // position
                        _PI_2,          // angle
                        scale,          // scale
                        glm::vec2( 6.0, 6.0 ),                          // uv scale
                        1.0,            // alpha
                        false,          // generate shadow
                        true,           // receiv shadow
                        1.0,            // shadow darkness
                        0.035,          // shadow bias
                        false,          // bloom
                        0.99,           // bloom bright value
                        false,          // opacity map
                        true,           // normal map
                        false,           // height map
                        0.12,           // displacement factor
                        0.15 );         // tessellation factor

  _grounds_type1.push_back( temp_object );


  // _walls type 11 object initialization ( room 3 walls )
  // -----------------------------------------------------
  for( int i = 0; i < 12; i ++ )
  {
    if( i >= 0 && i < 3 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += room3_offset_position;
      position += glm::vec3( 0.0, _wall_size, 0.0 );
      position -= glm::vec3( _wall_size - ( _wall_size * i ), 0.0, _wall_size );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0 , 0.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    if( i > 2 && i < 6 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += room3_offset_position;
      position += glm::vec3( _wall_size - ( _wall_size * ( i - 3 ) ), 0.0, _wall_size * 2.0 );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( -1.0, 0.0 , 0.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    if( i > 5 && i < 9 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += room3_offset_position;
      position += glm::vec3( _wall_size * 2.0, 0.0, _wall_size - ( _wall_size * ( i - 6 ) ) );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , 1.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    if( i > 8 && i < 12 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += room3_offset_position;
      position += glm::vec3( -_wall_size, _wall_size, _wall_size - ( _wall_size * ( i - 9 ) ) );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , -1.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    Object temp_object( 19, // ID
                        model_matrix,
                        position,
                        _PI_2,
                        glm::vec3( _wall_size, 1.0, _wall_size ),
                        glm::vec2( 6.0 / 3.0, 6.0 / 3.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        false,
                        0.99,
                        false,
                        true,
                        false,
                        0.12,
                        0.15 / 3.0 );

    if( i != 10 )
    {
      _walls_type1.push_back( temp_object );
    }
  }
}

void Scene::IBLInitialization()
{ 
  unsigned int capture_FBO;
  unsigned int capture_RBO;

  std::vector< std::string > hdr_texture_paths;
  hdr_texture_paths.push_back( std::string( "../Skybox/hdr skybox 2/Ridgecrest_Road_Ref.hdr" ) );
  hdr_texture_paths.push_back( std::string( "../Skybox/hdr skybox 1/Arches_E_PineTree_3k.hdr" ) );
  hdr_texture_paths.push_back( std::string( "../Skybox/hdr skybox 3/QueenMary_Chimney_Ref.hdr" ) );

  for( unsigned int cube_map_it = 0; cube_map_it < hdr_texture_paths.size(); cube_map_it++ )
  {
    unsigned int hdr_texture;
    unsigned int env_cubemap;
    unsigned int irradiance_cubemap;
    unsigned int pre_filter_cubemap;


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
    glGenFramebuffers( 1, &capture_FBO );
    glGenRenderbuffers( 1, &capture_RBO );
    glBindFramebuffer( GL_FRAMEBUFFER, capture_FBO );
    glBindRenderbuffer( GL_RENDERBUFFER, capture_RBO );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _res_env_cubeMap, _res_env_cubeMap );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_RBO );


    // Gen cubemap textures
    // --------------------
    glGenTextures( 1, &env_cubemap );
    glBindTexture( GL_TEXTURE_CUBE_MAP, env_cubemap );
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
    glBindFramebuffer( GL_FRAMEBUFFER, capture_FBO );
    for( unsigned int i = 0; i < 6; ++i )
    {
      glUniformMatrix4fv( glGetUniformLocation( _cube_map_converter_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( capture_view_matrices[ i ] ) );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, env_cubemap, 0 );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      _window->_toolbox->RenderCube();
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    glBindTexture( GL_TEXTURE_CUBE_MAP, env_cubemap );
    glGenerateMipmap( GL_TEXTURE_CUBE_MAP ); // generate mipmaps from first mip face (combatting visible dots artifact)

    
    // Gen & compute irradiance cube map textures
    // ------------------------------------------
    irradiance_cubemap = _window->_toolbox->GenIrradianceCubeMap( env_cubemap,
                                                                  _res_irradiance_cubeMap,
                                                                  _diffuse_irradiance_shader,
                                                                  _irradiance_sample_delta );

    
    // Gen & compute specular pre filter cube map textures
    // ---------------------------------------------------
    pre_filter_cubemap = _window->_toolbox->GenPreFilterCubeMap( env_cubemap,
                                                                 _res_pre_filter_cubeMap,
                                                                 _specular_pre_filter_shader,
                                                                 _pre_filter_sample_count,
                                                                 _pre_filter_max_mip_Level );


    // Gen specular pre brdf texture
    // -----------------------------    
    _pre_brdf_texture = _window->_toolbox->CreateEmptyTexture( _res_pre_brdf_texture,
                                                               _res_pre_brdf_texture,
                                                               GL_RG16F,
                                                               GL_RG,
                                                               false,
                                                               false,
                                                               0.0 );


    // Compute specular pre brdf texture
    // ---------------------------------
    
    // Re configure capture FBO
    glBindFramebuffer( GL_FRAMEBUFFER, capture_FBO );
    glBindRenderbuffer( GL_RENDERBUFFER, capture_RBO );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _res_pre_brdf_texture, _res_pre_brdf_texture );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _pre_brdf_texture, 0 );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( 0, 0, _res_pre_brdf_texture, _res_pre_brdf_texture );
    
    _specular_pre_brdf_shader.Use();
    glUniform1ui( glGetUniformLocation( _specular_pre_brdf_shader._program, "uSampleCount" ), _pre_brdf_sample_count );
    _window->_toolbox->RenderQuad();

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );


    // Save texture IDs
    // ----------------
    _hdr_textures.push_back( hdr_texture );
    _env_cubeMaps.push_back( env_cubemap);
    _irradiance_cubeMaps.push_back( irradiance_cubemap );
    _pre_filter_cubeMaps.push_back( pre_filter_cubemap );
  }
}

void Scene::ShadersInitialization()
{

  // Set and compile shaders
  // -----------------------
  _forward_pbr_shader.SetShaderClassicPipeline(         "../Shaders/forward_pbr_lighting.vs", "../Shaders/forward_pbr_lighting.fs" );
  _skybox_shader.SetShaderClassicPipeline(              "../Shaders/skybox.vs",               "../Shaders/skybox.fs" );
  _flat_color_shader.SetShaderClassicPipeline(          "../Shaders/flat_color.vs",           "../Shaders/flat_color.fs" );
  _observer_shader.SetShaderClassicPipeline(            "../Shaders/observer.vs",             "../Shaders/observer.fs" );
  _blur_shader.SetShaderClassicPipeline(                "../Shaders/observer.vs",             "../Shaders/blur.fs" );
  _post_process_shader.SetShaderClassicPipeline(        "../Shaders/observer.vs",             "../Shaders/post_process.fs" );
  _MS_blit_shader.SetShaderClassicPipeline(             "../Shaders/observer.vs",             "../Shaders/multisample_blit.fs" );
  _cube_map_converter_shader.SetShaderClassicPipeline(  "../Shaders/cube_map_converter.vs",   "../Shaders/cube_map_converter.fs" );
  _diffuse_irradiance_shader.SetShaderClassicPipeline(  "../Shaders/cube_map_converter.vs",   "../Shaders/IBL_diffuse_pre_irradiance.fs" );
  _specular_pre_filter_shader.SetShaderClassicPipeline( "../Shaders/cube_map_converter.vs",   "../Shaders/IBL_specular_pre_filter.fs" );
  _specular_pre_brdf_shader.SetShaderClassicPipeline(   "../Shaders/observer.vs",             "../Shaders/IBL_specular_pre_brdf.fs" );
  _forward_displacement_pbr_shader.SetShaderTessellationPipeline( "../Shaders/tessellation.vs",
                                                                  "../Shaders/tessellation.cs",
                                                                  "../Shaders/tessellation.es",
                                                                  "../Shaders/forward_pbr_lighting.fs" );
  _point_shadow_depth_shader.SetShaderGeometryPipeline( "../Shaders/point_shadow_depth.vs",
                                                        "../Shaders/point_shadow_depth.gs",
                                                        "../Shaders/point_shadow_depth.fs" );

  _geometry_pass_shader.SetShaderClassicPipeline(       "../Shaders/deferred_geometry_pass.vs", "../Shaders/deferred_geometry_pass.fs" );
  _lighting_pass_shader.SetShaderClassicPipeline(       "../Shaders/flat_color.vs",             "../Shaders/deferred_lighting_pass.fs" );
  _empty_shader.SetShaderClassicPipeline(               "../Shaders/flat_color.vs",             "../Shaders/empty.fs" );


  // Set texture uniform location
  // ----------------------------
  _forward_pbr_shader.Use();
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureAlbedo1" ),    0 ) ;
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureNormal1" ),    1 );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureHeight1" ),    2 ) ;
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureAO1" ),        3 );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureRoughness1" ), 4 );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureMetalness1" ), 5 );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureOpacity1" ),   6 );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIrradianceCubeMap" ), 7 );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uPreFilterCubeMap" ),  8 );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uPreBrdfLUT" ),        9 );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uDepthCubeMap" ),      10 ); 
  glUseProgram( 0 );

  _forward_displacement_pbr_shader.Use();
  glUniform1i( glGetUniformLocation( _forward_displacement_pbr_shader._program, "uTextureAlbedo1" ),    0 ) ;
  glUniform1i( glGetUniformLocation( _forward_displacement_pbr_shader._program, "uTextureNormal1" ),    1 );
  glUniform1i( glGetUniformLocation( _forward_displacement_pbr_shader._program, "uTextureHeight1" ),    2 ) ;
  glUniform1i( glGetUniformLocation( _forward_displacement_pbr_shader._program, "uTextureAO1" ),        3 );
  glUniform1i( glGetUniformLocation( _forward_displacement_pbr_shader._program, "uTextureRoughness1" ), 4 );
  glUniform1i( glGetUniformLocation( _forward_displacement_pbr_shader._program, "uTextureMetalness1" ), 5 );
  glUniform1i( glGetUniformLocation( _forward_displacement_pbr_shader._program, "uTextureOpacity1" ),   6 );
  glUniform1i( glGetUniformLocation( _forward_displacement_pbr_shader._program, "uIrradianceCubeMap" ), 7 );
  glUniform1i( glGetUniformLocation( _forward_displacement_pbr_shader._program, "uPreFilterCubeMap" ),  8 );
  glUniform1i( glGetUniformLocation( _forward_displacement_pbr_shader._program, "uPreBrdfLUT" ),        9 ); 
  glUniform1i( glGetUniformLocation( _forward_displacement_pbr_shader._program, "uDepthCubeMap" ),      10 ); 
  glUseProgram( 0 );

  _geometry_pass_shader.Use();
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureAlbedo1" ),    0 ) ;
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureNormal1" ),    1 );
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureHeight1" ),    2 ) ;
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureAO1" ),        3 );
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureRoughness1" ), 4 );
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureMetalness1" ), 5 );
  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uTextureSpecular1" ),  6 );
  glUseProgram( 0 );

  _lighting_pass_shader.Use();
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uGbufferPositionAndBloom" ),         0 ) ;
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uGbufferNormalAndBloomBrightness" ), 1 );
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uGbufferAlbedo" ),                   2 ) ;
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uGbufferRougnessMetalnessAO" ),      3 );
  glUniform1i( glGetUniformLocation( _lighting_pass_shader._program, "uIrradianceCubeMap" ),               4 ); 
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
  _table_model = new Model( "../Models/cube/cube.fbx", 
                            0, 
                            "Table1",
                            true,
                            false,
                            this );
  _table_model->PrintInfos();

  // Volume sphere model loading
  _sphere_model = new Model( "../Models/volume_sphere/volume_sphere.obj", 
                             1, 
                             "VolumeSphere",
                             false,
                             false,
                             this );
  _sphere_model->PrintInfos();

  // Ink bottle model loading
  _ink_bottle_model = new Model( "../Models/ink_bottle/ink_bottle.FBX", 
                                 2, 
                                 "InkBottle",
                                 _ink_bottle._normal_map,
                                 _ink_bottle._height_map,
                                 this );
  _ink_bottle_model->PrintInfos();

  // revolving door model loading
  _revolving_door_model = new Model( "../Models/revolving_door/RevolvingDoor.FBX", 
                                     3, 
                                     "RevolvingDoor",
                                     _revolving_door[ 0 ]._normal_map,
                                     _revolving_door[ 0 ]._height_map,
                                     this );
  _revolving_door_model->PrintInfos();
}

void Scene::TesselationInitialization()
{ 
  // Set size of the input patch
  glPatchParameteri( GL_PATCH_VERTICES, _tess_patch_vertices_count );
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

void Scene::SceneDepthPass()
{
  glm::mat4 model_matrix;


  // Create depth cubemap transformation matrices
  // --------------------------------------------
  glm::mat4 shadow_projection_matrix = glm::perspective( glm::radians( 90.0f ), 
                                                         (float)_depth_cubemap_res / (float)_depth_cubemap_res,
                                                         _shadow_near,
                                                         _shadow_far );
  std::vector< glm::mat4 > shadow_transform_matrices;
  shadow_transform_matrices.push_back( shadow_projection_matrix * glm::lookAt( _lights[ 0 ]._position, _lights[ 0 ]._position + glm::vec3( 1.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) ) );
  shadow_transform_matrices.push_back( shadow_projection_matrix * glm::lookAt( _lights[ 0 ]._position, _lights[ 0 ]._position + glm::vec3( -1.0f, 0.0f, 0.0f ), glm::vec3 (0.0f, -1.0f, 0.0f ) ) );
  shadow_transform_matrices.push_back( shadow_projection_matrix * glm::lookAt( _lights[ 0 ]._position, _lights[ 0 ]._position + glm::vec3( 0.0f, 1.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) ) );
  shadow_transform_matrices.push_back( shadow_projection_matrix * glm::lookAt( _lights[ 0 ]._position, _lights[ 0 ]._position + glm::vec3( 0.0f, -1.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, -1.0f ) ) );
  shadow_transform_matrices.push_back( shadow_projection_matrix * glm::lookAt( _lights[ 0 ]._position, _lights[ 0 ]._position + glm::vec3( 0.0f, 0.0f, 1.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) ) );
  shadow_transform_matrices.push_back( shadow_projection_matrix * glm::lookAt( _lights[ 0 ]._position, _lights[ 0 ]._position + glm::vec3( 0.0f, 0.0f, -1.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) ) );


  // Render scene to depth cubemap
  // -----------------------------
  glViewport( 0, 0, _depth_cubemap_res, _depth_cubemap_res );
  glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_depth_map_FBO );
  glClear( GL_DEPTH_BUFFER_BIT );

  _point_shadow_depth_shader.Use();   

  // Send each shadow transform matrix to render into each cube map face
  for( unsigned int i = 0; i < 6; i++ )
  {
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, std::string( "uShadowTransformMatrices[" + std::to_string( i ) + "]" ).data() ), 1, GL_FALSE, glm::value_ptr( shadow_transform_matrices[ i ] ) );
  }
  glUniform1f( glGetUniformLocation( _point_shadow_depth_shader._program, "uShadowFar" ), _shadow_far );
  glUniform3fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uLightPosition" ), 1, &_lights[ 0 ]._position[ 0 ] );


  // Draw ink bottle depth
  // ---------------------
  model_matrix = _ink_bottle._model_matrix;
  glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
  _ink_bottle_model->DrawDepth( _point_shadow_depth_shader, model_matrix );


  // Draw revolving door depth
  // -------------------------
  model_matrix = _revolving_door[ 0 ]._model_matrix;
  glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
  _revolving_door_model->DrawDepth( _point_shadow_depth_shader, model_matrix );


  glUseProgram( 0 );
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void Scene::SceneForwardRendering()
{

  Shader * current_shader;
  glm::mat4 model_matrix;


  // Bind correct buffer for drawing
  // -------------------------------
  glViewport( 0, 0, _window->_width, _window->_height );
  glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_temp_hdr_FBO );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


  // Draw skybox
  // -----------
  glDepthMask( GL_FALSE ); // desactivé juste pour draw la skybox
  _skybox_shader.Use();   
  glm::mat4 skybox_view_matrix = glm::mat4( glm::mat3( _camera->_view_matrix ) );  // Remove any translation component of the view matrix

  model_matrix = glm::mat4( 1.0f );
  glUniformMatrix4fv( glGetUniformLocation( _skybox_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _skybox_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( skybox_view_matrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _skybox_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
  glUniform1f( glGetUniformLocation( _skybox_shader._program, "uAlpha" ), 1.0 );

  glUniform1i( glGetUniformLocation( _skybox_shader._program, "uBloom" ), false );
  glUniform1f( glGetUniformLocation( _skybox_shader._program, "uBloomBrightness" ), 1.0 );

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, _env_cubeMaps[ _current_env ] ); 
  //glBindTexture( GL_TEXTURE_CUBE_MAP, _irradiance_cubeMaps[ _current_env ] ); 
  //glBindTexture( GL_TEXTURE_CUBE_MAP, _pre_filter_cubeMaps[ _current_env ] );

  _window->_toolbox->RenderCube();

  glDepthMask( GL_TRUE );  // réactivé pour draw le reste
  glUseProgram( 0 );


  // Draw lamps
  // ----------
  _flat_color_shader.Use();

  for( int i = 0; i < _lights.size(); i++ )
  {
    model_matrix= glm::mat4();
    model_matrix = glm::translate( model_matrix, _lights[ i ]._position );
    model_matrix = glm::scale( model_matrix, glm::vec3( 0.04f ) ); 
    glm::vec3 lamp_color = _lights[ i ]._color * _lights[ i ]._intensity;
    glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uViewMatrix" ) , 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _flat_color_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniform3f( glGetUniformLocation( _flat_color_shader._program, "uColor" ), lamp_color.x, lamp_color.y, lamp_color.z );

    glUniform1i( glGetUniformLocation( _flat_color_shader._program, "uBloom" ), true );
    glUniform1f( glGetUniformLocation( _flat_color_shader._program, "uBloomBrightness" ), 1.0f );

    _sphere_model->Draw( _flat_color_shader, model_matrix );
  }
  glBindVertexArray( 0 );
  glUseProgram( 0 );


  // Draw grounds type 1
  // -------------------
  for( int ground_it = 0; ground_it < _grounds_type1.size(); ground_it ++ )
  {
    ( _grounds_type1[ ground_it ]._height_map == true ) ? current_shader = &_forward_displacement_pbr_shader : current_shader = &_forward_pbr_shader; 

    current_shader->Use();

    model_matrix = _grounds_type1[ ground_it ]._model_matrix; 
   
    // Textures binding
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, _tex_albedo_ground1 );  
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, _tex_normal_ground1 ); 
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, _tex_height_ground1 ); 
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture( GL_TEXTURE_2D, _tex_AO_ground1 ); 
    glActiveTexture( GL_TEXTURE4 );
    glBindTexture( GL_TEXTURE_2D, _tex_roughness_ground1 ); 
    glActiveTexture( GL_TEXTURE5 );
    glBindTexture( GL_TEXTURE_2D, _tex_metalness_ground1 ); 
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _irradiance_cubeMaps[ _current_env ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _pre_filter_cubeMaps[ _current_env ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture );
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( current_shader->_program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( current_shader->_program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( current_shader->_program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );

    glUniform3fv( glGetUniformLocation( current_shader->_program, "uViewPos" ), 1, &_camera->_position[ 0 ] );

    // Point lights uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightCount" ), _lights.size() );
    for( int i = 0; i < _lights.size(); i++ )
    {
      string temp = to_string( i );
      glUniform3fv( glGetUniformLocation( current_shader->_program, ( "uLightPos[" + temp + "]" ).c_str() ),1, &_lights[ i ]._position[ 0 ] );
      glUniform3fv( glGetUniformLocation( current_shader->_program, ( "uLightColor[" + temp + "]" ).c_str() ),1, &_lights[ i ]._color[ 0 ] );
      glUniform1f(  glGetUniformLocation( current_shader->_program, ( "uLightIntensity[" + temp + "]" ).c_str() ), _lights[ i ]._intensity );
    }

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uBloom" ), _grounds_type1[ ground_it ]._bloom );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uBloomBrightness" ), _grounds_type1[ ground_it ]._bloom_brightness );

    glUniform1f( glGetUniformLocation( current_shader->_program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( current_shader->_program, "uAlpha" ), _grounds_type1[ ground_it ]._alpha );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uOpacityMap" ), _grounds_type1[ ground_it ]._opacity_map );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1f( glGetUniformLocation( current_shader->_program, "uDisplacementFactor" ), -_grounds_type1[ ground_it ]._displacement_factor );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uTessellationFactor" ), _grounds_type1[ ground_it ]._tessellation_factor );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uNormalMap" ), _grounds_type1[ ground_it ]._normal_map );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uReceivShadow" ), _grounds_type1[ ground_it ]._receiv_shadow );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightSourceIt" ), 0 );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowBias" ), _grounds_type1[ ground_it ]._shadow_bias );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowDarkness" ), _grounds_type1[ ground_it ]._shadow_darkness );

    glUniform1f( glGetUniformLocation( current_shader->_program, "uID" ), _grounds_type1[ ground_it ]._id );  

    glBindVertexArray( _ground1_VAO );
    ( _grounds_type1[ ground_it ]._height_map == true ) ? glDrawElements( GL_PATCHES, _ground1_indices.size(), GL_UNSIGNED_INT, 0 ) : glDrawElements( GL_TRIANGLES, _ground1_indices.size(), GL_UNSIGNED_INT, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
  }


  // Draw walls type 1
  // -----------------
  for( unsigned int wall_it = 0; wall_it < _walls_type1.size(); wall_it++ )
  {
    ( _walls_type1[ wall_it ]._height_map == true ) ? current_shader = &_forward_displacement_pbr_shader : current_shader = &_forward_pbr_shader; 

    current_shader->Use();

    model_matrix = _walls_type1[ wall_it ]._model_matrix;

    // Textures binding
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, _tex_albedo_ground1 );  
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, _tex_normal_ground1 ); 
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, _tex_height_ground1 ); 
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture( GL_TEXTURE_2D, _tex_AO_ground1 ); 
    glActiveTexture( GL_TEXTURE4 );
    glBindTexture( GL_TEXTURE_2D, _tex_roughness_ground1 ); 
    glActiveTexture( GL_TEXTURE5 );
    glBindTexture( GL_TEXTURE_2D, _tex_metalness_ground1 ); 
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _irradiance_cubeMaps[ _current_env ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _pre_filter_cubeMaps[ _current_env ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture );
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( current_shader->_program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( current_shader->_program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( current_shader->_program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );

    glUniform3fv( glGetUniformLocation( current_shader->_program, "uViewPos" ), 1, &_camera->_position[ 0 ] );

    // Point lights uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightCount" ), _lights.size() );
    for( int i = 0; i < _lights.size(); i++ )
    {
      string temp = to_string( i );
      glUniform3fv( glGetUniformLocation( current_shader->_program, ( "uLightPos[" + temp + "]" ).c_str() ),1, &_lights[ i ]._position[ 0 ] );
      glUniform3fv( glGetUniformLocation( current_shader->_program, ( "uLightColor[" + temp + "]" ).c_str() ),1, &_lights[ i ]._color[ 0 ] );
      glUniform1f(  glGetUniformLocation( current_shader->_program, ( "uLightIntensity[" + temp + "]" ).c_str() ), _lights[ i ]._intensity );
    }

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uBloom" ), _walls_type1[ wall_it ]._bloom );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uBloomBrightness" ), _walls_type1[ wall_it ]._bloom_brightness );

    glUniform1f( glGetUniformLocation( current_shader->_program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( current_shader->_program, "uAlpha" ), _walls_type1[ wall_it ]._alpha );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uOpacityMap" ), _walls_type1[ wall_it ]._opacity_map );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1f( glGetUniformLocation( current_shader->_program, "uDisplacementFactor" ), -_walls_type1[ wall_it ]._displacement_factor );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uTessellationFactor" ), _walls_type1[ wall_it ]._tessellation_factor );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uNormalMap" ), _walls_type1[ wall_it ]._normal_map );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uReceivShadow" ), _walls_type1[ wall_it ]._receiv_shadow );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightSourceIt" ), 0 );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowBias" ), _walls_type1[ wall_it ]._shadow_bias );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowDarkness" ), _walls_type1[ wall_it ]._shadow_darkness );

    glUniform1f( glGetUniformLocation( current_shader->_program, "uID" ), _walls_type1[ wall_it ]._id );  

    glBindVertexArray( _wall1_VAO );
    ( _walls_type1[ wall_it ]._height_map == true ) ? glDrawElements( GL_PATCHES, _wall1_indices.size(), GL_UNSIGNED_INT, 0 ) : glDrawElements( GL_TRIANGLES, _wall1_indices.size(), GL_UNSIGNED_INT, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
  }
  

  // Draw ink bottle
  // ---------------
  _forward_pbr_shader.Use();

  model_matrix = _ink_bottle._model_matrix;

  glActiveTexture( GL_TEXTURE7 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, _irradiance_cubeMaps[ _current_env ] );
  glActiveTexture( GL_TEXTURE8 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, _pre_filter_cubeMaps[ _current_env ] ); 
  glActiveTexture( GL_TEXTURE9 );
  glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
  glActiveTexture( GL_TEXTURE10 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

  // Matrices uniforms
  glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );

  glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewPos" ), 1, &_camera->_position[ 0 ] );

  // Point lights uniforms
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightCount" ), _lights.size() );
  for( int i = 0; i < _lights.size(); i++ )
  {
    string temp = to_string( i );
    glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, ( "uLightPos[" + temp + "]" ).c_str() ),1, &_lights[ i ]._position[ 0 ] );
    glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, ( "uLightColor[" + temp + "]" ).c_str() ),1, &_lights[ i ]._color[ 0 ] );
    glUniform1f(  glGetUniformLocation( _forward_pbr_shader._program, ( "uLightIntensity[" + temp + "]" ).c_str() ), _lights[ i ]._intensity );
  }

  // Bloom uniforms
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _ink_bottle._bloom );
  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _ink_bottle._bloom_brightness );

  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );

  // Opacity uniforms
  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _ink_bottle._alpha );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _ink_bottle._opacity_map );
  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
  
  // Displacement mapping uniforms
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _ink_bottle._normal_map );

  // Omnidirectional shadow mapping uniforms
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _ink_bottle._receiv_shadow );
  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), 0 );
  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _ink_bottle._shadow_bias );
  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _ink_bottle._shadow_darkness );

  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _ink_bottle._id );      

  _ink_bottle_model->Draw( _forward_pbr_shader, model_matrix );

  glUseProgram( 0 );


  // Draw revolving doors
  // --------------------
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  _forward_pbr_shader.Use();

  glActiveTexture( GL_TEXTURE7 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, _irradiance_cubeMaps[ _current_env ] );
  glActiveTexture( GL_TEXTURE8 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, _pre_filter_cubeMaps[ _current_env ] ); 
  glActiveTexture( GL_TEXTURE9 );
  glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
  glActiveTexture( GL_TEXTURE10 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

  // Matrices uniforms
  glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
  glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewPos" ), 1, &_camera->_position[ 0 ] );

  // Point lights uniforms
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightCount" ), _lights.size() );
  for( int i = 0; i < _lights.size(); i++ )
  {
    string temp = to_string( i );
    glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, ( "uLightPos[" + temp + "]" ).c_str() ),1, &_lights[ i ]._position[ 0 ] );
    glUniform3fv( glGetUniformLocation( _forward_pbr_shader._program, ( "uLightColor[" + temp + "]" ).c_str() ),1, &_lights[ i ]._color[ 0 ] );
    glUniform1f(  glGetUniformLocation( _forward_pbr_shader._program, ( "uLightIntensity[" + temp + "]" ).c_str() ), _lights[ i ]._intensity );
  }

  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );

  for( int door_it = 0; door_it < _revolving_door.size(); door_it++ )
  { 
    model_matrix = _revolving_door[ door_it ]._model_matrix;

    // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _revolving_door[ door_it ]._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _revolving_door[ door_it ]._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _revolving_door[ door_it ]._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _revolving_door[ door_it ]._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _revolving_door[ door_it ]._normal_map );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _revolving_door[ door_it ]._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), 0 );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _revolving_door[ door_it ]._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _revolving_door[ door_it ]._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _revolving_door[ door_it ]._id );      

    _revolving_door_model->Draw( _forward_pbr_shader, model_matrix );
  }

  glUseProgram( 0 );
  glDisable( GL_CULL_FACE );


  // Unbind current FBO
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
  model_matrix = glm::translate( model_matrix, _grounds_type1[ 0 ]._position );
  model_matrix = glm::rotate( model_matrix, _grounds_type1[ 0 ]._angle, glm::vec3( -1.0, 0.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, _grounds_type1[ 0 ]._scale ); 

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, _tex_albedo_ground1 );  
  glActiveTexture( GL_TEXTURE1 );
  glBindTexture( GL_TEXTURE_2D, _tex_normal_ground1 ); 
  glActiveTexture( GL_TEXTURE3 );
  glBindTexture( GL_TEXTURE_2D, _tex_AO_ground1 ); 
  glActiveTexture( GL_TEXTURE4 );
  glBindTexture( GL_TEXTURE_2D, _tex_roughness_ground1 ); 
  glActiveTexture( GL_TEXTURE5 );
  glBindTexture( GL_TEXTURE_2D, _tex_metalness_ground1 ); 

  glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( *iViewMatrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _geometry_pass_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( *iProjectionMatrix ) );

  glUniform1i( glGetUniformLocation( _geometry_pass_shader._program, "uBloom" ), _grounds_type1[ 0 ]._bloom );
  glUniform1f( glGetUniformLocation( _geometry_pass_shader._program, "uBloomBrightness" ), _grounds_type1[ 0 ]._bloom_brightness );

  glBindVertexArray( _ground1_VAO );
  glDrawArrays( GL_TRIANGLES, 0, 6 );
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

    //_sphere_model->Draw( _empty_shader, model_matrix );
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

    //_sphere_model->Draw( _lighting_pass_shader, model_matrix );
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

    //_sphere_model->Draw( _flat_color_shader, model_matrix );
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

      //_sphere_model->Draw( _flat_color_shader, model_matrix );
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

  // Bind correct FBO
  glViewport( 0, 0, _window->_width * _blur_downsample, _window->_height * _blur_downsample );
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
  // Bind correct FBO
  glViewport( 0, 0, _window->_width, _window->_height );
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

void Scene::AnimationsUpdate()
{ 

  // Revolving door rotation matrix update
  // -------------------------------------
  _door_rotation_matrix = glm::mat4();
  _door_rotation_matrix = glm::rotate( _door_rotation_matrix, _clock->GetCurrentTime() * 0.25f, glm::vec3( 0.0, 0.0, 1.0 ) );

  _door1_rotation_matrix = glm::mat4();
  _door1_rotation_matrix = glm::rotate( _door1_rotation_matrix, _door_angle, glm::vec3( 0.0, 0.0, -1.0 ) );

  _door2_rotation_matrix = glm::mat4();
  _door2_rotation_matrix = glm::rotate( _door2_rotation_matrix, _door_angle, glm::vec3( 0.0, 0.0, 1.0 ) );

  DoorOpeningScript();
}

void Scene::DoorOpeningScript()
{
  if( _door_open && _door_angle < 0.53 )
  {
    _door_angle += 0.1 * _clock->GetDeltaTime();
  }

  if( !_door_open )
  {
    _door_angle = 0.0;
  }
}

