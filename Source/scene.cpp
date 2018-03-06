#include "scene.hpp"
#include "window.hpp"


//******************************************************************************
//**********  Class Scene  *****************************************************
//******************************************************************************

Scene::Scene()
{ 
}

Scene::Scene( Window * iParentWindow )
{ 
  _pipeline_type = FORWARD_RENDERING;
  //_pipeline_type = DEFERRED_RENDERING;


  // Scene effects settings
  // ----------------------
  
  // near far
  _near        = 0.01;
  _far         = 30.0;
  _shadow_near = 0.01;
  _shadow_far  = 30.0;

  // Frame exposure
  _exposure           = 1.0;

  // Init bloom parameters
  _bloom              = true;
  _blur_downsample    = 1.0;
  _blur_pass_count    = 6;
  _blur_offset_factor = 1.8;

  // Init multi sample parameters
  _multi_sample    = false;
  _nb_multi_sample = 4;

  // Init IBL parameters
  _current_env              = 2;
  _res_env_cubemap          = 512;

  _res_irradiance_cubemap   = 32;
  _irradiance_sample_delta  = 0.025;
  
  _res_pre_filter_cubemap   = 256;
  _pre_filter_sample_count  = 1024 * 1;
  _pre_filter_max_mip_Level = 5;

  _res_pre_brdf_texture     = 512;
  _pre_brdf_sample_count    = 1024 * 1;    

  // Init tessellation parameters
  _tess_patch_vertices_count = 3;

  // Init omnidirectional shadow mapping parameters
  _depth_cubemap_res = 2048;

  // Lights volume
  _render_lights_volume = false;

  // Revolving door
  _door_angle          = 0.0;
  _revolving_door_open = false;

  // Simple door
  _door_position    = glm::vec3( 0.0 );
  _simple_door_open = false;

  _current_room                = 3;
  _current_shadow_light_source = 6;


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
  _camera = new Camera( glm::vec3( 17.2967, 0.514014, -17.9306 ),
                        glm::vec3( 0.724498, -0.409127, 0.554724 ),
                        glm::vec3( 0.0f, 1.0f,  0.0f ),
                        37.44,
                        -24.0,
                        _near,
                        _far,
                        45.0f,
                        ( float )_window->_width,
                        ( float )_window->_height,
                        2.0 );

  // Create and init all shaders
  ShadersInitialization(); 

  // Create all scene's objects
  ObjectsInitialization();

   // Create lights
  LightsInitialization();

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
  std::cout << "Scene's data initialization in progress..." << std::endl;

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


  // Create ground type 1 VAO
  // ------------------------
  _window->_toolbox->CreatePlaneVAO( &_ground2_VAO,
                                     &_ground2_VBO,
                                     &_ground2_IBO,
                                     &_ground2_indices,
                                     40,
                                     3.0 );


  // Create wall type 1 VAO
  // ----------------------
  _window->_toolbox->CreatePlaneVAO( &_wall1_VAO,
                                     &_wall1_VBO,
                                     &_wall1_IBO,
                                     &_wall1_indices,
                                     40,
                                     _walls_type1[ 0 ]._uv_scale.x );


  // Create wall type 2 VAO
  // ----------------------
  _window->_toolbox->CreatePlaneVAO( &_wall2_VAO,
                                     &_wall2_VBO,
                                     &_wall2_IBO,
                                     &_wall2_indices,
                                     40,
                                     _walls_type1[ 0 ]._uv_scale.x * 1.5 );


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


  // Load entrance floor material textures
  // -------------------------------------
  _loaded_materials.push_back( _window->_toolbox->LoadMaterialTextures( "entrance_floor",
                                                                        anisotropy_value,
                                                                        false ) );


  // Load entrance walls material textures
  // -------------------------------------
  _loaded_materials.push_back( _window->_toolbox->LoadMaterialTextures( "entrance_walls",
                                                                        anisotropy_value,
                                                                        false ) );


  // Load room1 walls material textures
  // ----------------------------------
  _loaded_materials.push_back( _window->_toolbox->LoadMaterialTextures( "room1_walls",
                                                                        anisotropy_value,
                                                                        false ) );


  // Load room1 floor material textures
  // ----------------------------------
  _loaded_materials.push_back( _window->_toolbox->LoadMaterialTextures( "room1_floor",
                                                                        anisotropy_value,
                                                                        false ) );


  // Load room1 roof material textures
  // ---------------------------------
  _loaded_materials.push_back( _window->_toolbox->LoadMaterialTextures( "room1_roof",
                                                                        anisotropy_value,
                                                                        true ) );


  // Load corridor1 floor material textures
  // --------------------------------------
  _loaded_materials.push_back( _window->_toolbox->LoadMaterialTextures( "corridor1_floor",
                                                                        anisotropy_value,
                                                                        false ) );


  // Load corridor1 walls material textures
  // --------------------------------------
  _loaded_materials.push_back( _window->_toolbox->LoadMaterialTextures( "corridor1_walls",
                                                                        anisotropy_value,
                                                                        true ) );


  // Load room2 roof & floor material textures
  // -----------------------------------------
  _loaded_materials.push_back( _window->_toolbox->LoadMaterialTextures( "room2_floor",
                                                                        anisotropy_value,
                                                                        false ) );


  // Load room2 walls material textures
  // ----------------------------------
  _loaded_materials.push_back( _window->_toolbox->LoadMaterialTextures( "room2_walls",
                                                                        anisotropy_value,
                                                                        true ) );


  // Load corridor2 walls material textures
  // --------------------------------------
  _loaded_materials.push_back( _window->_toolbox->LoadMaterialTextures( "corridor2_floor",
                                                                        anisotropy_value,
                                                                        false ) );


  // Load corridor2 walls material textures
  // --------------------------------------
  _loaded_materials.push_back( _window->_toolbox->LoadMaterialTextures( "corridor2_walls",
                                                                        anisotropy_value,
                                                                        false ) );


  // Load room3 floor material textures
  // ----------------------------------
  _loaded_materials.push_back( _window->_toolbox->LoadMaterialTextures( "room3_floor",
                                                                        anisotropy_value,
                                                                        false ) );


  // Load room3 walls material textures
  // ----------------------------------
  _loaded_materials.push_back( _window->_toolbox->LoadMaterialTextures( "room3_walls",
                                                                        anisotropy_value,
                                                                        false ) );

  std::cout << "Scene's data initialization done.\n" << std::endl;
}

void Scene::LightsInitialization()
{ 
  _lights.clear();
  
  PointLight::SetLightsMultiplier( 30.0 );

  
  _lights.push_back( PointLight( glm::vec3( -_ground_size * 1.15, _wall_size * 0.5, 0.0 ),
                                 glm::vec3( 1.0, 1.0, 1.0 ),
                                 0.2,
                                 3.0 ) );


  // Top lights
  // ----------
  _lights.push_back( PointLight( _top_light[ 0 ]._IBL_position + glm::vec3( 0.0, 0.2, 0.0 ),
                                 glm::vec3( 1.0, 1.0, 1.0 ),
                                 0.25,
                                 3.0 ) );

  _lights.push_back( PointLight( _top_light[ 1 ]._IBL_position + glm::vec3( 0.0, 0.2, 0.0 ),
                                 glm::vec3( 1.0, 1.0, 1.0 ),
                                 0.25,
                                 3.0 ) );

  _lights.push_back( PointLight( _top_light[ 4 ]._IBL_position + glm::vec3( 0.0, 0.2, 0.0 ),
                                 glm::vec3( 1.0, 1.0, 1.0 ),
                                 0.25,
                                 3.0 ) ); 

  _lights.push_back( PointLight( _top_light[ 5 ]._IBL_position + glm::vec3( 0.0, 0.2, 0.0 ),
                                 glm::vec3( 1.0, 1.0, 1.0 ),
                                 0.25,
                                 3.0 ) ); 

  _lights.push_back( PointLight( _top_light[ 2 ]._IBL_position + glm::vec3( 0.0, 0.2, 0.0 ),
                                 glm::vec3( 1.0, 1.0, 1.0 ),
                                 0.25,
                                 3.0 ) );  

  _lights.push_back( PointLight( _top_light[ 3 ]._IBL_position + glm::vec3( 0.0, 0.2, 0.0 ),
                                 glm::vec3( 1.0, 1.0, 1.0 ),
                                 0.25,
                                 3.0 ) ); 


  // Wall lights
  // -----------
  _lights.push_back( PointLight( _wall_light[ 0 ]._IBL_position + glm::vec3( 0.0, 0.0, -0.2 ),
                                 glm::vec3( 1.0, 0.835294118, 0.341176471 ),
                                 0.1,
                                 3.0 ) );

  _lights.push_back( PointLight( _wall_light[ 1 ]._IBL_position + glm::vec3( 0.0, 0.0, 0.2 ),
                                 glm::vec3( 1.0, 0.835294118, 0.341176471 ),
                                 0.1,
                                 3.0 ) );

  for( unsigned int i = 0; i < _lights.size(); i++ )
  {
    _lights[ i ]._intensity *= PointLight::GetLightsMultiplier();
  }
}

void Scene::ObjectsInitialization()
{ 
  std::cout << "Scene's objects initialization in progress..." << std::endl;

  glm::vec3 position;
  glm::vec3 scale;
  glm::vec3 IBL_position( 0.0, 0.0, 0.0 );
  glm::mat4 model_matrix = glm::mat4(); 
  _ground_size = 8.0;
  _wall_size   = _ground_size / 3.0;


  // _grounds type 1 object initialization ( room 1 ground & roof )
  // --------------------------------------------------------------
  for( int i = 0; i < 2; i ++ )
  {
    bool emissive = false;
    if( i == 1 )
    {
      emissive = true;
    }

    int material_id = 3;
   
    if( i == 1 )
    {
      material_id = 4;
    }

    position = glm::vec3( -(_ground_size * 0.5 ) + 0.0, 0.0, -(_ground_size * 0.5 ) + 0.0 );
    position += glm::vec3( _ground_size * i, i * _wall_size, 0.0 );
    IBL_position = position + glm::vec3( _ground_size * 0.5, 0.0, _ground_size * 0.5 );

    if( i == 1 )
    {
      IBL_position = position - glm::vec3( _ground_size * 0.5, 0.0, -_ground_size * 0.5 );
    }

    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, position );
    model_matrix = glm::rotate( model_matrix, ( float )_PI * ( float )i, glm::vec3( 0.0, 0.0 , 1.0 ) );
    glm::vec3 scale( _ground_size, 1.0, _ground_size );
    model_matrix = glm::scale( model_matrix, glm::vec3( _ground_size, 1.0, _ground_size ) ); 

    Object temp_object( 1,              // ID
                        model_matrix,   // model matrix
                        position,       // position
                        IBL_position,   // position use to generate IBL cubemaps
                        _PI_2,          // angle
                        scale,          // scale
                        glm::vec2( 6.0, 6.0 ),   // uv scale
                        1.0,            // alpha
                        false,          // generate shadow
                        true,           // receiv shadow
                        1.0,            // shadow darkness
                        0.035,          // shadow bias
                        false,          // bloom
                        0.99,           // bloom bright value
                        false,          // opacity map
                        true,           // normal map
                        false,          // height map
                        0.05,           // displacement factor
                        0.85,           // tessellation factor
                        material_id,    // material ID
                        false,          // emissive
                        5.0,            // emissive factor
                        true,           // need parallax cubemap
                        true );         // need IBL

    _grounds_type1.push_back( temp_object );
  }


  // _walls type 1 object initialization ( room 1 walls )
  // ----------------------------------------------------
  for( int i = 0; i < 12; i ++ )
  {
    IBL_position = glm::vec3( 0.0, _wall_size * 0.5, 0.0 );
   
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
      position += glm::vec3( ( _wall_size * 2.0 ), _wall_size, _wall_size - ( _wall_size * ( i - 6 ) ) );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0, 0.0 ) );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , 1.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    if( i > 8 && i < 12 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( -_wall_size, _wall_size, ( _wall_size * ( i - 9 ) ) );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0, 0.0 ) );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , -1.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    Object temp_object( 4, // ID
                        model_matrix,
                        position,
                        IBL_position,
                        _PI_2,
                        glm::vec3( _wall_size, 1.0, _wall_size ),
                        glm::vec2( 1.0, 1.0 ),
                        1.0,
                        false,
                        true,
                        0.5,
                        0.035,
                        false,
                        0.99,
                        false,
                        true,
                        true,
                        0.05,
                        0.1,
                        2,
                        false,
                        1.0,
                        true,
                        false );

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
    int material_id = 0;
    bool tessellation = false;
    bool IBL = true;
    bool parallax = false;

    if( i == 0 || i == 1 || i == 2 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ), 0.0, -( _wall_size * 0.5 ) );
      position += glm::vec3( -( _wall_size * ( 2.0 + i ) ), 0.0, 0.0 );
      IBL_position = glm::vec3( -( _wall_size * ( 2.0 + 1 ) ), 0.0, 0.0 );
                     
      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::scale( model_matrix, scale );

      parallax = true;
    }

    if( i == 3 || i == 4 || i == 5 )
    {
      material_id = 1;
      tessellation = true;
      IBL_position = glm::vec3( -( _wall_size * ( 2.0 + 1 ) ), _wall_size + 0.058, 0.0 );

      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( -_wall_size, _wall_size + 0.058, 0.0 );

      if( i == 4 )
      {
        position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
        position += glm::vec3( -_wall_size * 2, _wall_size + 0.058, 0.0 );
      }

      if( i == 5 )
      {
        position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
        position += glm::vec3( -_wall_size * 3, _wall_size + 0.058, 0.0 );
      }

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI, glm::vec3( 0.0, 0.0 , 1.0 ) );
      model_matrix = glm::scale( model_matrix, scale );
    }

    Object temp_object( 5, // ID
                        model_matrix,
                        position,
                        IBL_position,
                        _PI_2,
                        scale,
                        glm::vec2( 1.0, 1.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        false,
                        0.99,
                        false,
                        true,
                        tessellation,
                        0.08,
                        0.05,
                        material_id,
                        false,
                        1.0,
                        parallax,
                        IBL );

    _walls_type1.push_back( temp_object );
  }


  // _walls type 3 object initialization ( entrance corridor walls )
  // ---------------------------------------------------------------
  for( int i = 0; i < 7; i ++ )
  { 
    IBL_position = glm::vec3( -( _wall_size * ( 2.0 + 1 ) ), _wall_size * 0.5, 0.0 );
    glm::vec3 scale( _wall_size, 1.0, _wall_size );

    if( i == 0 || i == 1 || i == 2 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( -_wall_size * ( 2.0 + i ), _wall_size, -0.07 );
      
      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0 , 0.0 ) );
      model_matrix = glm::scale( model_matrix, scale );
    }

    if( i == 3 || i == 4 || i == 5 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( -_wall_size * ( 2.0 + ( i - 3 ) ), 0.0, _wall_size + 0.07 );

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

    Object temp_object( 5, // ID
                        model_matrix,
                        position,
                        IBL_position,
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
                        true,
                        0.08,
                        0.05,
                        1,
                        false,
                        1.0,
                        false,
                        true );

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
      IBL_position = position + glm::vec3( 0.0, _wall_size * 0.5, 0.0 );
    }

    if( i == 1 )
    {
      position = glm::vec3( 0.0, 0.125, -13.8 );
      IBL_position = position + glm::vec3( 0.0, _wall_size * 0.5, 0.0 );
    }

    if( i == 2 )
    {
      position = glm::vec3( 13.8, 0.07, -_ground_size - ( _wall_size * 4 ) );
      IBL_position = position + glm::vec3( 0.0, _wall_size * 0.5, 0.0 );
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
                                       IBL_position,
                                       0.0,
                                       scale,
                                       glm::vec2( 1.0, 1.0 ),
                                       1.0,
                                       true,
                                       true,
                                       1.0,
                                       0.015,
                                       true,
                                       0.70,
                                       true,
                                       true,
                                       false,
                                       0.0,
                                       0.0,
                                       0,
                                       false,
                                       1.0,
                                       false,
                                       true ) );
  }


  // _walls type 4 object initialization ( corridor room 1 to room 2 ground )
  // ------------------------------------------------------------------------
  for( int i = 0; i < 4; i ++ )
  { 
    glm::vec3 scale( _wall_size, 1.0, _wall_size );

    position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
    position += glm::vec3( 0.0, -0.07, -_wall_size * ( 2 + i ) );
    IBL_position = glm::vec3( 0.0, -0.07, -_wall_size * ( 2 + 2 ) + _wall_size * 0.5 );
    
    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, position );
    model_matrix = glm::scale( model_matrix, scale );

    Object temp_object( 8, // ID
                        model_matrix,
                        position,
                        IBL_position,
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
                        true,
                        0.20,
                        0.06,
                        5,
                        false,
                        1.0,
                        true,
                        true );

    _walls_type1.push_back( temp_object );
  }


  // _walls type 5 object initialization ( corridor room 1 to room 2 roof )
  // ----------------------------------------------------------------------
  for( int i = 0; i < 4; i ++ )
  { 
    glm::vec3 scale( _wall_size, 1.0, _wall_size );

    position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
    position += glm::vec3( 0.0, _wall_size + 0.07, -_wall_size * ( i + 1 ) );
    IBL_position = glm::vec3( 0.0, _wall_size + 0.07, -_wall_size * ( 2 + 1 ) - _wall_size * 0.5 );
    
    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, position );
    model_matrix = glm::rotate( model_matrix, ( float )_PI, glm::vec3( 1.0, 0.0 , 0.0 ) );
    model_matrix = glm::scale( model_matrix, scale );

    Object temp_object( 9, // ID
                        model_matrix,
                        position,
                        IBL_position,
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
                        true,
                        0.2,
                        0.06,
                        5,
                        false,
                        1.0,
                        true,
                        true );

    _walls_type1.push_back( temp_object );
  }


  // _walls type 6 object initialization ( corridor room 1 to room 2 walls )
  // -----------------------------------------------------------------------
  for( int i = 0; i < 8; i ++ )
  { 
    glm::vec3 scale( _wall_size, 1.0, _wall_size );
    IBL_position = glm::vec3( 0.0, _wall_size * 0.5, -_wall_size * ( 2 + 1 ) - _wall_size * 0.5 );

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
                        IBL_position,
                        _PI_2,
                        scale,
                        glm::vec2( 6.0 / 3.0, 6.0 / 3.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        true,
                        0.99,
                        false,
                        true,
                        false,
                        0.12,
                        0.15 / 3.0,
                        6,
                        true,
                        20.0,
                        true,
                        true );

    _walls_type1.push_back( temp_object );
  }


  // _grounds type 2 object initialization ( room 2 ground )
  // ------------------------------------------------------
  scale = glm::vec3( _ground_size, 1.0, _ground_size );

  position = glm::vec3( -(_ground_size * 0.5 ) + 0.0, 0.0, -(_ground_size * 0.5 ) + 0.0 );
  position += glm::vec3( 0.0, 0.0, -_ground_size - ( _wall_size * 4 ) );
  IBL_position = position + glm::vec3( (_ground_size * 0.5 ), 0.0, (_ground_size * 0.5 ) ); 
  
  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::scale( model_matrix, glm::vec3( _ground_size, 1.0, _ground_size ) ); 

  Object temp_object( 11,              // ID
                      model_matrix,   // model matrix
                      position,       // position
                      IBL_position,
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
                      false,          // height map
                      0.12,           // displacement factor
                      0.15,           // tessellation factor
                      7,
                      false,
                      1.0,
                      true,
                      true );         

  _grounds_type1.push_back( temp_object );


  // _grounds type 3 object initialization ( room 2 roof )
  // -----------------------------------------------------
  scale = glm::vec3( _ground_size, 1.0, _ground_size );

  position = glm::vec3( -(_ground_size * 0.5 ) + 0.0, 0.0, -(_ground_size * 0.5 ) + 0.0 );
  position += glm::vec3( 0.0, _wall_size, -_wall_size * 4 );
  IBL_position = position + glm::vec3( _ground_size * 0.5, 0.0, -_ground_size * 0.5 );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI, glm::vec3( 1.0, 0.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, glm::vec3( _ground_size, 1.0, _ground_size ) );

  temp_object = Object( 12,              // ID
                        model_matrix,   // model matrix
                        position,       // position
                        IBL_position,
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
                        false,          // height map
                        0.12,           // displacement factor
                        0.15,           // tessellation factor
                        7,
                        false,
                        1.0,
                        true,
                        true );         

  _grounds_type1.push_back( temp_object );


  // _walls type 7 object initialization ( room 2 walls )
  // ----------------------------------------------------
  glm::vec3 room2_offset_position( 0.0, 0.0, -_ground_size - (_wall_size * 4 ) );
  for( int i = 0; i < 12; i ++ )
  {
    IBL_position = glm::vec3( 0.0, _wall_size * 0.5, -_ground_size - ( _wall_size * 4 ) );

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
      position += glm::vec3( ( _wall_size * ( i - 3 ) ) - _wall_size, 0.0, _wall_size * 2.0 );
      position += room2_offset_position;

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( -1.0, 0.0 , 0.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    if( i > 5 && i < 9 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( _wall_size * 2.0, _wall_size, _wall_size - ( _wall_size * ( i - 6 ) ) );
      position += room2_offset_position;

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0, 0.0 ) );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , 1.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    if( i > 8 && i < 12 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += glm::vec3( -_wall_size, _wall_size, ( _wall_size * ( i - 9 ) ) );
      position += room2_offset_position;

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0, 0.0 ) );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , -1.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    Object temp_object( 13, // ID
                        model_matrix,
                        position,
                        IBL_position,
                        _PI_2,
                        glm::vec3( _wall_size, 1.0, _wall_size ),
                        glm::vec2( 6.0 / 3.0, 6.0 / 3.0 ),
                        1.0,
                        false,
                        true,
                        1.0,
                        0.035,
                        true,
                        0.99,
                        false,
                        true,
                        false,
                        0.12,
                        0.15 / 3.0,
                        8,
                        true,
                        17.0,
                        false,
                        true );

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
    position += glm::vec3( _wall_size * ( 2 + i ), -0.02, 0.0 );
    IBL_position = room2_offset_position + glm::vec3( _wall_size * ( 2 + 1.5 ), -0.02, 0.0 );
        
    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, position );
    model_matrix = glm::scale( model_matrix, scale );

    Object temp_object( 14, // ID
                        model_matrix,
                        position,
                        IBL_position,
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
                        true,
                        0.06,
                        0.2,
                        9,
                        false,
                        1.0,
                        true,
                        true );

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
    IBL_position = room2_offset_position + glm::vec3( _wall_size * ( 2 + 1.5 ), _wall_size, 0.0 );

    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, position );
    model_matrix = glm::rotate( model_matrix, ( float )_PI, glm::vec3( 0.0, 0.0 , 1.0 ) );
    model_matrix = glm::scale( model_matrix, scale );

    Object temp_object( 15, // ID
                        model_matrix,
                        position,
                        IBL_position,
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
                        0.03,
                        0.2,
                        9,
                        false,
                        1.0,
                        true,
                        true );

    _walls_type1.push_back( temp_object );
  }


  // _walls type 10 object initialization ( corridor room 2 to room 3 walls )
  // ------------------------------------------------------------------------
  for( int i = 0; i < 8; i ++ )
  { 
    glm::vec3 scale( _wall_size, 1.0, _wall_size );

    IBL_position = room2_offset_position + glm::vec3( _wall_size * ( 2 + 1.5 ), _wall_size * 0.5, 0.0 );

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
                        IBL_position,
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
                        0.1,
                        0.1,
                        10,
                        false,
                        1.0,
                        true,
                        true );

    _walls_type1.push_back( temp_object );
  }


  // _grounds type 4 object initialization ( room 3 ground )
  // -------------------------------------------------------
  glm::vec3 room3_offset_position = room2_offset_position + glm::vec3( _ground_size + ( _wall_size * 4 ), 0.0, 0.0 );

  scale = glm::vec3( _ground_size, 1.0, _ground_size );

  position = glm::vec3( -(_ground_size * 0.5 ) + 0.0, 0.0, -(_ground_size * 0.5 ) + 0.0 );
  position += room3_offset_position;
  IBL_position = room3_offset_position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::scale( model_matrix, glm::vec3( _ground_size, 1.0, _ground_size ) ); 

  temp_object = Object( 17,              // ID
                        model_matrix,   // model matrix
                        position,       // position
                        IBL_position,
                        _PI_2,          // angle
                        scale,          // scale
                        glm::vec2( 6.0, 6.0 ),     // uv scale
                        1.0,            // alpha
                        false,          // generate shadow
                        true,           // receiv shadow
                        1.0,            // shadow darkness
                        0.035,          // shadow bias
                        false,          // bloom
                        0.99,           // bloom bright value
                        false,          // opacity map
                        true,           // normal map
                        true,           // height map
                        0.08,          // displacement factor
                        0.1,
                        11,
                        false,
                        1.0,
                        true,
                        true );         

  _grounds_type1.push_back( temp_object );


  // _grounds type 5 object initialization ( room 3 roof )
  // -----------------------------------------------------
  scale = glm::vec3( _ground_size, 1.0, _ground_size );

  position = glm::vec3( -(_ground_size * 0.5 ) + 0.0, 0.0, -(_ground_size * 0.5 ) + 0.0 );
  position += room3_offset_position;
  position += glm::vec3( 0.0, _wall_size, _ground_size );
  IBL_position = room3_offset_position + glm::vec3( 0.0, _wall_size * 0.95, 0.0 );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI, glm::vec3( 1.0, 0.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, glm::vec3( _ground_size, 1.0, _ground_size ) ); 

  temp_object = Object( 18,              // ID
                        model_matrix,   // model matrix
                        position,       // position
                        IBL_position,
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
                        false,          // height map
                        0.12,           // displacement factor
                        0.15,
                        0,
                        false,
                        17.0,
                        true,
                        true );        

  _grounds_type1.push_back( temp_object );


  // _walls type 11 object initialization ( room 3 walls )
  // -----------------------------------------------------
  for( int i = 0; i < 12; i ++ )
  { 
    IBL_position = room3_offset_position + glm::vec3( 0.0, _wall_size * 0.5, 0.0 );
  
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
      position += glm::vec3( _wall_size * 2.0, _wall_size, _wall_size - ( _wall_size * ( i - 6 ) ) );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0 , 0.0 ) );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , 1.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    if( i > 8 && i < 12 )
    {
      position = glm::vec3( -( _wall_size * 0.5 ) + 0.0, 0.0, -( _wall_size * 0.5 ) + 0.0 );
      position += room3_offset_position;
      position += glm::vec3( -_wall_size, _wall_size, ( _wall_size * ( i - 9 ) ) );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0 , 0.0 ) );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0 , -1.0 ) );
      model_matrix = glm::scale( model_matrix, glm::vec3( _wall_size, 1.0, _wall_size ) ); 
    }

    Object temp_object( 19, // ID
                        model_matrix,
                        position,
                        IBL_position,
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
                        0.15 / 3.0,
                        12,
                        false,
                        1.0,
                        true,
                        true );

    if( i != 10 )
    {
      _walls_type1.push_back( temp_object );
    }
  }


  // _simple_door object initialization
  // ----------------------------------
  for( int i = 0; i < 2; i ++ )
  {   
    scale = glm::vec3( 1.9, 2.2, 1.9 );

    if( i == 0 )
    {
      position = glm::vec3( 0.0, 0.05, ( -_ground_size * 0.5 ) + 0.08 );
      IBL_position = position + glm::vec3( 0.0, _wall_size * 0.5, 0.0 );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::scale( model_matrix, scale );
    }

    if( i == 1 )
    {
      position = glm::vec3( _ground_size * 0.5, 0.05, -_ground_size - _wall_size * 4.0 );  
      IBL_position = position + glm::vec3( 0.0, _wall_size * 0.5, 0.0 );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 1.0, 0.0 ) );
      model_matrix = glm::scale( model_matrix, scale );
    }

    _simple_door.push_back( Object( 20,              // ID
                            model_matrix,   // model matrix
                            position,       // position
                            IBL_position,   // position use to generate IBL cubemaps
                            _PI_2,          // angle
                            scale,          // scale
                            glm::vec2( 6.0, 6.0 ),   // uv scale
                            1.0,            // alpha
                            true,           // generate shadow
                            true,           // receiv shadow
                            1.0,            // shadow darkness
                            0.035,          // shadow bias
                            false,          // bloom
                            0.99,           // bloom bright value
                            false,          // opacity map
                            true,           // normal map
                            false,          // height map
                            0.05,           // displacement factor
                            0.85,           // tessellation factor
                            0,              // material ID
                            false,          // emissive
                            5.0,            // emissive factor
                            true,           // need parallax cubemap
                            true ) );       // need IBL
  }


  // _top_light objects initialization
  // ---------------------------------
  for( int i = 0; i < 6; i ++ )
  {   
    scale = glm::vec3( 0.016, 0.016, 0.016 );

    if( i == 0 )
    {
      position = glm::vec3( 1.5, _wall_size , 0.0 );
    }

    if( i == 1 )
    {
      position = glm::vec3( -1.5, _wall_size , 0.0 );
    }

    if( i == 2 )
    {
      position = glm::vec3( 1.5, _wall_size , 0.0 );
      position += room3_offset_position;
    }

    if( i == 3 )
    {
      position = glm::vec3( -1.5, _wall_size , 0.0 );
      position += room3_offset_position;
    }

    if( i == 4 )
    {
      position = glm::vec3( 1.5, _wall_size , -1.0 );
      position += room2_offset_position;
    }

    if( i == 5 )
    {
      position = glm::vec3( -1.5, _wall_size , 1.0 );
      position += room2_offset_position;
    }
  
    IBL_position = position + glm::vec3( 0.0, -0.7, 0.0 );

    model_matrix = glm::mat4();
    model_matrix = glm::translate( model_matrix, position );
    model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0, 0.0 ) );
    model_matrix = glm::scale( model_matrix, scale );

    _top_light.push_back( Object( 21,             // ID
                                  model_matrix,   // model matrix
                                  position,       // position
                                  IBL_position,   // position use to generate IBL cubemaps
                                  _PI_2,          // angle
                                  scale,          // scale
                                  glm::vec2( 6.0, 6.0 ),   // uv scale
                                  1.0,            // alpha
                                  false,           // generate shadow
                                  true,          // receiv shadow
                                  1.0,            // shadow darkness
                                  0.035,          // shadow bias
                                  true,          // bloom
                                  0.99,           // bloom bright value
                                  false,          // opacity map
                                  true,           // normal map
                                  false,          // height map
                                  0.05,           // displacement factor
                                  0.85,           // tessellation factor
                                  0,              // material ID
                                  true,          // emissive
                                  15.0,           // emissive factor
                                  false,          // need parallax cubemap
                                  true ) );       // need IBL
  }


  // _wall_light objects initialization
  // ----------------------------------
  for( int i = 0; i < 2; i ++ )
  {   
    scale = glm::vec3( 0.05, 0.05, 0.05 );

    if( i == 0 )
    {
      position = glm::vec3( _ground_size * 0.95, 0.5, -_wall_size * 0.46 ) + room2_offset_position;
      
      IBL_position = position + glm::vec3( 0.0, 0.0, 0.2 );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( -1.0, 0.0, 0.0 ) );
      model_matrix = glm::scale( model_matrix, scale );
    }

    if( i == 1 )
    {
      position = glm::vec3( _ground_size * 1.35, 0.5, _wall_size * 0.46 ) + room2_offset_position;
      
      IBL_position = position + glm::vec3( 0.0, 0.0, -0.2 );

      model_matrix = glm::mat4();
      model_matrix = glm::translate( model_matrix, position );
      model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0, 0.0 ) );
      model_matrix = glm::scale( model_matrix, scale );
    }

    _wall_light.push_back( Object( 22,             // ID
                                   model_matrix,   // model matrix
                                   position,       // position
                                   IBL_position,   // position use to generate IBL cubemaps
                                   _PI_2,          // angle
                                   scale,          // scale
                                   glm::vec2( 6.0, 6.0 ),   // uv scale
                                   1.0,            // alpha
                                   false,          // generate shadow
                                   false,          // receiv shadow
                                   1.0,            // shadow darkness
                                   0.035,          // shadow bias
                                   true,          // bloom
                                   0.99,           // bloom bright value
                                   false,          // opacity map
                                   true,           // normal map
                                   false,          // height map
                                   0.05,           // displacement factor
                                   0.85,           // tessellation factor
                                   0,              // material ID
                                   true,           // emissive
                                   15.0,           // emissive factor
                                   false,          // need parallax cubemap
                                   true ) );       // need IBL
  }


  // _room1_table1 object initialization
  // -----------------------------------
  scale = glm::vec3( 0.8 );
  position = glm::vec3( 2.5, 0.43, 2.5 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2 * ( float )0.5, glm::vec3( 0.0, 1.0, 0.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _room1_table1.Set( Object( 23,             // ID
                             model_matrix,   // model matrix
                             position,       // position
                             IBL_position,   // position use to generate IBL cubemaps
                             _PI_2,          // angle
                             scale,          // scale
                             glm::vec2( 6.0, 6.0 ),   // uv scale
                             1.0,            // alpha
                             true,          // generate shadow
                             true,           // receiv shadow
                             1.0,            // shadow darkness
                             0.035,          // shadow bias
                             false,          // bloom
                             0.99,           // bloom bright value
                             false,          // opacity map
                             true,           // normal map
                             false,          // height map
                             0.05,           // displacement factor
                             0.85,           // tessellation factor
                             0,              // material ID
                             false,          // emissive
                             15.0,           // emissive factor
                             false,          // need parallax cubemap
                             true ) );       // need IBL


  // _bottle object initialization
  // -----------------------------
  scale = glm::vec3( 0.0015 );
  position = _room1_table1._position + glm::vec3( -0.15, 0.43, -0.1 );
  IBL_position = position + glm::vec3( 0.0, 0.1, 0.0 );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( -1.0, 0.0, 0.0 ) );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0, 1.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _bottle.Set( Object( 24,             // ID
                       model_matrix,   // model matrix
                       position,       // position
                       IBL_position,   // position use to generate IBL cubemaps
                       _PI_2,          // angle
                       scale,          // scale
                       glm::vec2( 6.0, 6.0 ),   // uv scale
                       1.0,            // alpha
                       true,           // generate shadow
                       false,          // receiv shadow
                       1.0,            // shadow darkness
                       0.035,          // shadow bias
                       false,          // bloom
                       0.5,            // bloom bright value
                       false,          // opacity map
                       true,           // normal map
                       false,          // height map
                       0.05,           // displacement factor
                       0.85,           // tessellation factor
                       0,              // material ID
                       false,          // emissive
                       15.0,           // emissive factor
                       false,          // need parallax cubemap
                       true ) );       // need IBL


  // _ball object initialization
  // ---------------------------
  scale = glm::vec3( 0.0135 );
  position = _room1_table1._position + glm::vec3( 0.3, 0.525, -0.5 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, -1.0, 0.0 ) );
  model_matrix = glm::rotate( model_matrix, ( float )_PI, glm::vec3( -1.0, 0.0, 0.0 ) );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 1.0, 0.0, 0.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _ball.Set( Object( 25,             // ID
                     model_matrix,   // model matrix
                     position,       // position
                     IBL_position,   // position use to generate IBL cubemaps
                     _PI_2,          // angle
                     scale,          // scale
                     glm::vec2( 6.0, 6.0 ),   // uv scale
                     1.0,            // alpha
                     true,           // generate shadow
                     false,          // receiv shadow
                     1.0,            // shadow darkness
                     0.035,          // shadow bias
                     false,          // bloom
                     0.5,            // bloom bright value
                     false,          // opacity map
                     true,           // normal map
                     false,          // height map
                     0.1,          // displacement factor
                     0.3,           // tessellation factor
                     0,              // material ID
                     false,          // emissive
                     15.0,           // emissive factor
                     false,          // need parallax cubemap
                     true ) );       // need IBL


  // _box_bag object initialization
  // ---------------------------
  scale = glm::vec3( 0.012, 0.012, 0.012 );
  position = glm::vec3( _ground_size * 0.25, 1.05, -1.75 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( -1.0, 0.0, 0.0 ) );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 0.0, 1.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _box_bag.Set( Object( 26,             // ID
                        model_matrix,   // model matrix
                        position,       // position
                        IBL_position,   // position use to generate IBL cubemaps
                        _PI_2,          // angle
                        scale,          // scale
                        glm::vec2( 6.0, 6.0 ),   // uv scale
                        1.0,            // alpha
                        true,           // generate shadow
                        false,          // receiv shadow
                        1.0,            // shadow darkness
                        0.035,          // shadow bias
                        false,          // bloom
                        0.5,            // bloom bright value
                        false,          // opacity map
                        true,           // normal map
                        false,          // height map
                        0.1,            // displacement factor
                        0.3,            // tessellation factor
                        0,              // material ID
                        false,          // emissive
                        15.0,           // emissive factor
                        false,          // need parallax cubemap
                        true ) );       // need IBL


  // _chest object initialization
  // ---------------------------
  scale = glm::vec3( 0.65 );
  position = glm::vec3( 2.7, 0.3, 0.0 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 1.0, 0.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _chest.Set( Object( 27,             // ID
                      model_matrix,   // model matrix
                      position,       // position
                      IBL_position,   // position use to generate IBL cubemaps
                      _PI_2,          // angle
                      scale,          // scale
                      glm::vec2( 6.0, 6.0 ),   // uv scale
                      1.0,            // alpha
                      true,           // generate shadow
                      false,          // receiv shadow
                      1.0,            // shadow darkness
                      0.035,          // shadow bias
                      false,          // bloom
                      0.5,            // bloom bright value
                      false,          // opacity map
                      true,           // normal map
                      false,          // height map
                      0.1,            // displacement factor
                      0.3,            // tessellation factor
                      0,              // material ID
                      false,          // emissive
                      15.0,           // emissive factor
                      false,          // need parallax cubemap
                      true ) );      // need IBL


  // _sofa object initialization
  // ---------------------------
  scale = glm::vec3( 1.75 );
  position = glm::vec3( -1.35, 0.95, 1.9 );
  IBL_position = position - glm::vec3( 0.0, 0.45, 0.0 );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI * ( float )1.0, glm::vec3( 0.0, -1.0, 0.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _sofa.Set( Object( 28,             // ID
                     model_matrix,   // model matrix
                     position,       // position
                     IBL_position,   // position use to generate IBL cubemaps
                     _PI_2,          // angle
                     scale,          // scale
                     glm::vec2( 6.0, 6.0 ),   // uv scale
                     1.0,            // alpha
                     true,           // generate shadow
                     true,          // receiv shadow
                     1.0,            // shadow darkness
                     0.035,          // shadow bias
                     false,          // bloom
                     0.5,            // bloom bright value
                     false,          // opacity map
                     true,           // normal map
                     false,          // height map
                     0.1,            // displacement factor
                     0.3,            // tessellation factor
                     0,              // material ID
                     false,          // emissive
                     15.0,           // emissive factor
                     false,          // need parallax cubemap
                     true ) );      // need IBL


  // _sack object initialization
  // ---------------------------
  scale = glm::vec3( 0.005 );
  position = _sofa._position + glm::vec3( -0.5, -0.425, 0.08 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::scale( model_matrix, scale );

  _sack.Set( Object( 29,             // ID
                       model_matrix,   // model matrix
                       position,       // position
                       IBL_position,   // position use to generate IBL cubemaps
                       _PI_2,          // angle
                       scale,          // scale
                       glm::vec2( 6.0, 6.0 ),   // uv scale
                       1.0,            // alpha
                       true,           // generate shadow
                       false,          // receiv shadow
                       1.0,            // shadow darkness
                       0.035,          // shadow bias
                       false,          // bloom
                       0.5,            // bloom bright value
                       false,          // opacity map
                       true,           // normal map
                       false,          // height map
                       0.1,            // displacement factor
                       0.3,            // tessellation factor
                       0,              // material ID
                       false,          // emissive
                       15.0,           // emissive factor
                       false,          // need parallax cubemap
                       true ) );      // need IBL


  // _room1_table2 object initialization
  // ---------------------------
  scale = glm::vec3( 0.5 );
  position = glm::vec3( 0.4, 0.27, -1.0 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::scale( model_matrix, scale );

  _room1_table2.Set( Object( 30,             // ID
                             model_matrix,   // model matrix
                             position,       // position
                             IBL_position,   // position use to generate IBL cubemaps
                             _PI_2,          // angle
                             scale,          // scale
                             glm::vec2( 6.0, 6.0 ),   // uv scale
                             1.0,            // alpha
                             true,           // generate shadow
                             true,           // receiv shadow
                             1.0,            // shadow darkness
                             0.013,          // shadow bias
                             false,          // bloom
                             0.5,            // bloom bright value
                             false,          // opacity map
                             true,           // normal map
                             false,          // height map
                             0.1,            // displacement factor
                             0.3,            // tessellation factor
                             0,              // material ID
                             false,          // emissive
                             15.0,           // emissive factor
                             false,          // need parallax cubemap
                             true ) );       // need IBL


  // _ink_bottle object initialization
  // ---------------------------------
  scale = glm::vec3( 0.011, 0.011, 0.011 );

  position = _room1_table2._position + glm::vec3( -1.2, 0.345, 0.3 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2 * ( float )1.5, glm::vec3( 0.0, 1.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _ink_bottle.Set( Object( 2, // ID
                           model_matrix,
                           position,
                           IBL_position,
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
                           0.0,
                           -1,
                           false,
                           1.0,
                           false,
                           true ) );


  // _book object initialization
  // ---------------------------
  scale = glm::vec3( 0.008 );
  position = _room1_table2._position + glm::vec3( -0.9, 0.27, 0.0 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI, glm::vec3( 0.0, 1.0 , 0.0 ) );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( -1.0, 0.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _book.Set( Object( 31,             // ID
                     model_matrix,   // model matrix
                     position,       // position
                     IBL_position,   // position use to generate IBL cubemaps
                     _PI_2,          // angle
                     scale,          // scale
                     glm::vec2( 6.0, 6.0 ),   // uv scale
                     1.0,            // alpha
                     true,           // generate shadow
                     true,          // receiv shadow
                     1.0,            // shadow darkness
                     0.035,          // shadow bias
                     false,          // bloom
                     0.5,            // bloom bright value
                     false,          // opacity map
                     true,           // normal map
                     false,          // height map
                     0.1,            // displacement factor
                     0.3,            // tessellation factor
                     0,              // material ID
                     false,          // emissive
                     15.0,           // emissive factor
                     false,          // need parallax cubemap
                     true ) );      // need IBL


  // _radio object initialization
  // ---------------------------
  scale = glm::vec3( 0.12 );
  position = _room1_table2._position + glm::vec3( -1.35, 0.35, -0.25 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2 * ( float )0.65, glm::vec3( 0.0, 1.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _radio.Set( Object( 32,             // ID
                      model_matrix,   // model matrix
                      position,       // position
                      IBL_position,   // position use to generate IBL cubemaps
                      _PI_2,          // angle
                      scale,          // scale
                      glm::vec2( 6.0, 6.0 ),   // uv scale
                      1.0,            // alpha
                      true,           // generate shadow
                      true,          // receiv shadow
                      1.0,            // shadow darkness
                      0.035,          // shadow bias
                      false,          // bloom
                      0.5,            // bloom bright value
                      false,          // opacity map
                      true,           // normal map
                      false,          // height map
                      0.1,            // displacement factor
                      0.3,            // tessellation factor
                      0,              // material ID
                      false,          // emissive
                      15.0,           // emissive factor
                      false,          // need parallax cubemap
                      true ) );      // need IBL


  // _screen object initialization
  // -----------------------------
  scale = glm::vec3( 1.3 );
  position = room2_offset_position + glm::vec3( -2.25, 0.975, -2.25 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2 * ( float )0.5, glm::vec3( 0.0, 1.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _screen.Set( Object( 33,             // ID
                      model_matrix,   // model matrix
                      position,       // position
                      IBL_position,   // position use to generate IBL cubemaps
                      _PI_2,          // angle
                      scale,          // scale
                      glm::vec2( 6.0, 6.0 ),   // uv scale
                      1.0,            // alpha
                      true,           // generate shadow
                      false,          // receiv shadow
                      1.0,            // shadow darkness
                      0.035,          // shadow bias
                      true,          // bloom
                      0.99,            // bloom bright value
                      true,          // opacity map
                      true,           // normal map
                      false,          // height map
                      0.1,            // displacement factor
                      0.3,            // tessellation factor
                      0,              // material ID
                      true,           // emissive
                      7.0,           // emissive factor
                      false,          // need parallax cubemap
                      true ) );       // need IBL


  // _bike object initialization
  // -----------------------------
  scale = glm::vec3( 0.0067 );
  position = room2_offset_position + glm::vec3( 1.4, -0.005, -2.0 );
  IBL_position = position + + glm::vec3( 0.0, 0.8, 0.0 );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( -1.0, 0.0 , 0.0 ) );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2 * ( float )0.75, glm::vec3( 0.0, 0.0 , -1.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _bike.Set( Object( 34,             // ID
                     model_matrix,   // model matrix
                     position,       // position
                     IBL_position,   // position use to generate IBL cubemaps
                     _PI_2,          // angle
                     scale,          // scale
                     glm::vec2( 6.0, 6.0 ),   // uv scale
                     1.0,            // alpha
                     true,           // generate shadow
                     false,          // receiv shadow
                     1.0,            // shadow darkness
                     0.035,          // shadow bias
                     false,          // bloom
                     0.5,            // bloom bright value
                     true,          // opacity map
                     true,           // normal map
                     false,          // height map
                     0.1,            // displacement factor
                     0.3,            // tessellation factor
                     0,              // material ID
                     false,           // emissive
                     17.0,           // emissive factor
                     false,          // need parallax cubemap
                     true ) );       // need IBL


  // _pilar object initialization
  // -----------------------------
  scale = glm::vec3( 1.35 );
  position = room2_offset_position + glm::vec3( 0.0, 1.35, 0.0 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::scale( model_matrix, scale );

  _pilar.Set( Object( 35,             // ID
                      model_matrix,   // model matrix
                      position,       // position
                      IBL_position,   // position use to generate IBL cubemaps
                      _PI_2,          // angle
                      scale,          // scale
                      glm::vec2( 6.0, 6.0 ),   // uv scale
                      1.0,            // alpha
                      true,           // generate shadow
                      true,          // receiv shadow
                      1.0,            // shadow darkness
                      0.015,          // shadow bias
                      false,          // bloom
                      0.5,            // bloom bright value
                      false,          // opacity map
                      true,           // normal map
                      false,          // height map
                      0.1,            // displacement factor
                      0.3,            // tessellation factor
                      0,              // material ID
                      false,          // emissive
                      17.0,           // emissive factor
                      false,          // need parallax cubemap
                      true ) );       // need IBL


  // _scanner object initialization
  // -----------------------------
  scale = glm::vec3( 0.013 );
  position = room2_offset_position + glm::vec3( 2.8, 0.0, 0.0 );
  IBL_position = position + glm::vec3( 0.0, _wall_size * 0.5, 0.0 );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 1.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _scanner.Set( Object( 36,             // ID
                        model_matrix,   // model matrix
                        position,       // position
                        IBL_position,   // position use to generate IBL cubemaps
                        _PI_2,          // angle
                        scale,          // scale
                        glm::vec2( 6.0, 6.0 ),   // uv scale
                        1.0,            // alpha
                        true,           // generate shadow
                        true,          // receiv shadow
                        1.0,            // shadow darkness
                        0.02,          // shadow bias
                        true,          // bloom
                        0.9,            // bloom bright value
                        false,          // opacity map
                        true,           // normal map
                        false,          // height map
                        0.1,            // displacement factor
                        0.3,            // tessellation factor
                        0,              // material ID
                        true,           // emissive
                        10,             // emissive factor
                        false,          // need parallax cubemap
                        true ) );       // need IBL


  // _room2_table1 object initialization
  // -----------------------------------
  scale = glm::vec3( 1.0 );
  position = room2_offset_position + glm::vec3( -2.0, 0.52, 2.0 );
  IBL_position = position + glm::vec3( 0.0, 0.28, 0.0 );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2 * ( float )0.5, glm::vec3( 0.0, -1.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _room2_table1.Set( Object( 37,             // ID
                             model_matrix,   // model matrix
                             position,       // position
                             IBL_position,   // position use to generate IBL cubemaps
                             _PI_2,          // angle
                             scale,          // scale
                             glm::vec2( 6.0, 6.0 ),   // uv scale
                             1.0,            // alpha
                             true,           // generate shadow
                             true,           // receiv shadow
                             1.0,            // shadow darkness
                             0.015,          // shadow bias
                             false,          // bloom
                             0.5,            // bloom bright value
                             false,          // opacity map
                             true,           // normal map
                             false,          // height map
                             0.1,            // displacement factor
                             0.3,            // tessellation factor
                             0,              // material ID
                             false,          // emissive
                             17.0,           // emissive factor
                             false,          // need parallax cubemap
                             true ) );       // need IBL


  // _mask object initialization
  // ---------------------------
  scale = glm::vec3( 0.3 );
  position = _room2_table1._position + glm::vec3( 0.45, 0.75, 0.25 );
  IBL_position = position + glm::vec3( 0.0, 0.3, 0.0 );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2 * ( float ) 1.65, glm::vec3( 0.0, 1.0 , 0.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _mask.Set( Object( 38,             // ID
                     model_matrix,   // model matrix
                     position,       // position
                     IBL_position,   // position use to generate IBL cubemaps
                     _PI_2,          // angle
                     scale,          // scale
                     glm::vec2( 6.0, 6.0 ),   // uv scale
                     1.0,            // alpha
                     true,           // generate shadow
                     false,           // receiv shadow
                     1.0,            // shadow darkness
                     0.015,          // shadow bias
                     false,          // bloom
                     0.5,            // bloom bright value
                     false,          // opacity map
                     true,           // normal map
                     false,          // height map
                     0.1,            // displacement factor
                     0.3,            // tessellation factor
                     0,              // material ID
                     false,          // emissive
                     17.0,           // emissive factor
                     false,          // need parallax cubemap
                     true ) );       // need IBL


  // _arm object initialization
  // --------------------------
  scale = glm::vec3( 0.01 );
  position = _room2_table1._position + glm::vec3( 0.22, 0.38, 0.25 );
  IBL_position = position + glm::vec3( -0.25, 0.4, -0.35 );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2 * ( float )0.7, glm::vec3( 0.0, 1.0, 0.0 ) );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2 * ( float ) 0.1, glm::vec3( 0.0, 0.0, 1.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _arm.Set( Object( 39,             // ID
                    model_matrix,   // model matrix
                    position,       // position
                    IBL_position,   // position use to generate IBL cubemaps
                    _PI_2,          // angle
                    scale,          // scale
                    glm::vec2( 6.0, 6.0 ),   // uv scale
                    1.0,            // alpha
                    true,           // generate shadow
                    true,           // receiv shadow
                    1.0,            // shadow darkness
                    0.015,          // shadow bias
                    false,          // bloom
                    0.5,            // bloom bright value
                    false,          // opacity map
                    true,           // normal map
                    false,          // height map
                    0.1,            // displacement factor
                    0.3,            // tessellation factor
                    0,              // material ID
                    false,          // emissive
                    17.0,           // emissive factor
                    false,          // need parallax cubemap
                    true ) );       // need IBL


  // _tank object initialization
  // --------------------------
  scale = glm::vec3( 1.5 );
  position = room3_offset_position + glm::vec3( 2.4, 0.7, -0.5 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::scale( model_matrix, scale );

  _tank.Set( Object( 40,             // ID
                     model_matrix,   // model matrix
                     position,       // position
                     IBL_position,   // position use to generate IBL cubemaps
                     _PI_2,          // angle
                     scale,          // scale
                     glm::vec2( 6.0, 6.0 ),   // uv scale
                     1.0,            // alpha
                     true,           // generate shadow
                     true,          // receiv shadow
                     1.0,            // shadow darkness
                     0.015,          // shadow bias
                     false,          // bloom
                     0.5,            // bloom bright value
                     false,          // opacity map
                     true,           // normal map
                     false,          // height map
                     0.1,            // displacement factor
                     0.3,            // tessellation factor
                     0,              // material ID
                     false,          // emissive
                     17.0,           // emissive factor
                     false,          // need parallax cubemap
                     true ) );       // need IBL


  // _shelving object initialization
  // -------------------------------
  scale = glm::vec3( 0.01 );
  position = room3_offset_position + glm::vec3( 0.7, 0.90, 0.7 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2 * ( float ) 0.1, glm::vec3( 0.0, 1.0, 0.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _shelving.Set( Object( 41,             // ID
                         model_matrix,   // model matrix
                         position,       // position
                         IBL_position,   // position use to generate IBL cubemaps
                         _PI_2,          // angle
                         scale,          // scale
                         glm::vec2( 6.0, 6.0 ),   // uv scale
                         1.0,            // alpha
                         true,           // generate shadow
                         true,          // receiv shadow
                         1.0,            // shadow darkness
                         0.007,          // shadow bias
                         false,          // bloom
                         0.5,            // bloom bright value
                         false,          // opacity map
                         true,           // normal map
                         false,          // height map
                         0.1,            // displacement factor
                         0.3,            // tessellation factor
                         0,              // material ID
                         false,          // emissive
                         17.0,           // emissive factor
                         false,          // need parallax cubemap
                         true ) );       // need IBL


  // _gun1 object initialization
  // ---------------------------
  scale = glm::vec3( 0.003 );
  position = _shelving._position + glm::vec3( -0.03, -0.125, 0.3 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI, glm::vec3( 1.0, 0.0, 0.0 ) );
  model_matrix = glm::rotate( model_matrix, ( float )0.4, glm::vec3( 0.0, 1.0, 0.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _gun1.Set( Object( 42,             // ID
                     model_matrix,   // model matrix
                     position,       // position
                     IBL_position,   // position use to generate IBL cubemaps
                     _PI_2,          // angle
                     scale,          // scale
                     glm::vec2( 6.0, 6.0 ),   // uv scale
                     1.0,            // alpha
                     true,           // generate shadow
                     false,           // receiv shadow
                     1.0,            // shadow darkness
                     0.015,          // shadow bias
                     false,          // bloom
                     0.5,            // bloom bright value
                     false,          // opacity map
                     true,           // normal map
                     false,          // height map
                     0.1,            // displacement factor
                     0.3,            // tessellation factor
                     0,              // material ID
                     false,          // emissive
                     17.0,           // emissive factor
                     false,          // need parallax cubemap
                     true ) );       // need IBL


  // _gun2 object initialization
  // ---------------------------
  scale = glm::vec3( 0.5 );
  position = _shelving._position + glm::vec3( -0.1, 0.453, 0.0 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2, glm::vec3( 0.0, 1.0, 0.0 ) );
  model_matrix = glm::rotate( model_matrix, ( float )0.04, glm::vec3( 0.0, 0.0, 1.0 ) );
  model_matrix = glm::scale( model_matrix, scale );

  _gun2.Set( Object( 43,             // ID
                     model_matrix,   // model matrix
                     position,       // position
                     IBL_position,   // position use to generate IBL cubemaps
                     _PI_2,          // angle
                     scale,          // scale
                     glm::vec2( 6.0, 6.0 ),   // uv scale
                     1.0,            // alpha
                     true,           // generate shadow
                     false,          // receiv shadow
                     1.0,            // shadow darkness
                     0.015,          // shadow bias
                     false,          // bloom
                     0.5,            // bloom bright value
                     false,          // opacity map
                     true,           // normal map
                     false,          // height map
                     0.1,            // displacement factor
                     0.3,            // tessellation factor
                     0,              // material ID
                     false,          // emissive
                     17.0,           // emissive factor
                     false,          // need parallax cubemap
                     true ) );       // need IBL


  // _gun3 object initialization
  // ---------------------------
  scale = glm::vec3( 0.0018 );
  position = _shelving._position + glm::vec3( -0.04, 1.295, 0.75 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )0.21, glm::vec3( -1.0, 0.0, 0.0 ) ); 
  model_matrix = glm::scale( model_matrix, scale );

  _gun3.Set( Object( 44,             // ID
                     model_matrix,   // model matrix
                     position,       // position
                     IBL_position,   // position use to generate IBL cubemaps
                     _PI_2,          // angle
                     scale,          // scale
                     glm::vec2( 6.0, 6.0 ),   // uv scale
                     1.0,            // alpha
                     true,           // generate shadow
                     false,          // receiv shadow
                     1.0,            // shadow darkness
                     0.015,          // shadow bias
                     false,          // bloom
                     0.5,            // bloom bright value
                     false,          // opacity map
                     true,           // normal map
                     false,          // height map
                     0.1,            // displacement factor
                     0.3,            // tessellation factor
                     0,              // material ID
                     false,          // emissive
                     17.0,           // emissive factor
                     false,          // need parallax cubemap
                     true ) );       // need IBL


  // _room3_table1 object initialization
  // -----------------------------------
  scale = glm::vec3( 0.0022 );
  position = room3_offset_position + glm::vec3( -0.7, 0.0, 2.5 );
  IBL_position = position + glm::vec3( 0.0, 0.5, 0.0 );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2 * ( float )1.15, glm::vec3( 0.0, 1.0, 0.0 ) ); 
  model_matrix = glm::scale( model_matrix, scale );

  _room3_table1.Set( Object( 45,             // ID
                             model_matrix,   // model matrix
                             position,       // position
                             IBL_position,   // position use to generate IBL cubemaps
                             _PI_2,          // angle
                             scale,          // scale
                             glm::vec2( 6.0, 6.0 ),   // uv scale
                             1.0,            // alpha
                             true,           // generate shadow
                             false,          // receiv shadow
                             1.0,            // shadow darkness
                             0.015,          // shadow bias
                             false,          // bloom
                             0.5,            // bloom bright value
                             false,          // opacity map
                             true,           // normal map
                             false,          // height map
                             0.1,            // displacement factor
                             0.3,            // tessellation factor
                             0,              // material ID
                             false,          // emissive
                             17.0,           // emissive factor
                             false,          // need parallax cubemap
                             true ) );       // need IBL


  // _room3_table2 object initialization
  // -----------------------------------
  scale = glm::vec3( 0.0045 );
  position = room3_offset_position + glm::vec3( -2.2, 0.8, 2.2 );
  IBL_position = position;

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::rotate( model_matrix, ( float )_PI_2 * ( float )0.3, glm::vec3( 0.0, -1.0, 0.0 ) ); 
  model_matrix = glm::scale( model_matrix, scale );

  _room3_table2.Set( Object( 46,             // ID
                             model_matrix,   // model matrix
                             position,       // position
                             IBL_position,   // position use to generate IBL cubemaps
                             _PI_2,          // angle
                             scale,          // scale
                             glm::vec2( 6.0, 6.0 ),   // uv scale
                             1.0,            // alpha
                             true,           // generate shadow
                             false,          // receiv shadow
                             1.0,            // shadow darkness
                             0.015,          // shadow bias
                             false,          // bloom
                             0.5,            // bloom bright value
                             false,          // opacity map
                             true,           // normal map
                             false,          // height map
                             0.1,            // displacement factor
                             0.3,            // tessellation factor
                             0,              // material ID
                             false,          // emissive
                             17.0,           // emissive factor
                             false,          // need parallax cubemap
                             true ) );       // need IBL


  // _helmet object initialization
  // -----------------------------
  scale = glm::vec3( 0.002 );
  position = _room3_table1._position + glm::vec3( 0.2, 1.0, 0.0 );
  IBL_position = position + glm::vec3( 0.0, 0.3, 0.0 );

  model_matrix = glm::mat4();
  model_matrix = glm::translate( model_matrix, position );
  model_matrix = glm::scale( model_matrix, scale );

  _helmet.Set( Object( 47,             // ID
                       model_matrix,   // model matrix
                       position,       // position
                       IBL_position,   // position use to generate IBL cubemaps
                       _PI_2,          // angle
                       scale,          // scale
                       glm::vec2( 6.0, 6.0 ),   // uv scale
                       1.0,            // alpha
                       true,           // generate shadow
                       false,          // receiv shadow
                       1.0,            // shadow darkness
                       0.015,          // shadow bias
                       false,          // bloom
                       0.5,            // bloom bright value
                       false,          // opacity map
                       true,           // normal map
                       false,          // height map
                       0.1,            // displacement factor
                       0.3,            // tessellation factor
                       0,              // material ID
                       false,          // emissive
                       17.0,           // emissive factor
                       false,          // need parallax cubemap
                       true ) );       // need IBL


  std::cout << "Scene's objects initialization done.\n" << std::endl;
}

void Scene::IBLInitialization()
{ 
  std::cout << "Scene's IBL initialization in progress..." << std::endl;

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
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _res_env_cubemap, _res_env_cubemap );
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
      _res_env_cubemap, 
      _res_env_cubemap, 
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

    glViewport( 0, 0, _res_env_cubemap, _res_env_cubemap ); // don't forget to configure the viewport to the capture dimensions.
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
                                                                  _res_irradiance_cubemap,
                                                                  _diffuse_irradiance_shader,
                                                                  _irradiance_sample_delta );

    
    // Gen & compute specular pre filter cube map textures
    // ---------------------------------------------------
    pre_filter_cubemap = _window->_toolbox->GenPreFilterCubeMap( env_cubemap,
                                                                 _res_pre_filter_cubemap,
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

  std::cout << "Scene's IBL initialization done.\n" << std::endl;
}

void Scene::ShadersInitialization()
{
  std::cout << "Scene's shaders initialization in progress..." << std::endl;


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
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uTextureEmissive1" ),  11 ); 
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

  std::cout << "Scene's shaders initialization done.\n" << std::endl;
}

void Scene::ModelsLoading()
{ 
  std::cout << "Scene's models loading in progress..." << std::endl;

  Model::SetToolbox( _window->_toolbox );
  Model::SetScene( this );

  // Volume sphere model loading
  _sphere_model = new Model( "../Models/volume_sphere/volume_sphere.obj", 
                             1, 
                             "VolumeSphere",
                             false,
                             false );
  _sphere_model->PrintInfos();

  // revolving door model loading
  _revolving_door_model = new Model( "../Models/revolving_door/RevolvingDoor.FBX", 
                                     3, 
                                     "RevolvingDoor",
                                     _revolving_door[ 0 ]._normal_map,
                                     _revolving_door[ 0 ]._height_map );
  _revolving_door_model->PrintInfos();

  // simple door model loading
  _simple_door_model = new Model( "../Models/simple_door/FinalPortalDoor.fbx", 
                                  4, 
                                  "SimpleDoor",
                                  _simple_door[ 0 ]._normal_map,
                                  _simple_door[ 0 ]._height_map );
  _simple_door_model->PrintInfos();

  // top light model loading
  _top_light_model = new Model( "../Models/top_light/top_light.fbx", 
                                  5, 
                                  "TopLight",
                                  _top_light[ 0 ]._normal_map,
                                  _top_light[ 0 ]._height_map );
  _top_light_model->PrintInfos();

  // wall light model loading
  _wall_light_model = new Model( "../Models/wall_light/wall_light.fbx", 
                                  6, 
                                  "WallLight",
                                  _wall_light[ 0 ]._normal_map,
                                  _wall_light[ 0 ]._height_map );
  _wall_light_model->PrintInfos();

  // Ink bottle model loading
  /*_ink_bottle_model = new Model( "../Models/ink_bottle/ink_bottle.FBX", 
                                 2, 
                                 "InkBottle",
                                 _ink_bottle._normal_map,
                                 _ink_bottle._height_map );
  _ink_bottle_model->PrintInfos();

  // room1 table1 model loading
  _room1_table1_model = new Model( "../Models/room1_table1/room1_table1.dae", 
                                   7, 
                                   "Room1Table1",
                                   _room1_table1._normal_map,
                                   _room1_table1._height_map );
  _room1_table1_model->PrintInfos();

  // bottle model loading
  _bottle_model = new Model( "../Models/bottle/bottle.FBX", 
                             8, 
                             "Bottle",
                             _bottle._normal_map,
                             _bottle._height_map );
  _bottle_model->PrintInfos();

  // ball model loading
  _ball_model = new Model( "../Models/ball/ball.FBX", 
                           9, 
                           "Ball",
                           _ball._normal_map,
                           _ball._height_map );
  _ball_model->PrintInfos();

  // box_bag model loading
  _box_bag_model = new Model( "../Models/box_bag/box_bag.fbx", 
                           10, 
                           "box_bag",
                           _box_bag._normal_map,
                           _box_bag._height_map );
  _box_bag_model->PrintInfos();

  // _chest model loading
  _chest_model = new Model( "../Models/chest/chest.dae", 
                            11, 
                            "chest",
                            _chest._normal_map,
                            _chest._height_map );
  _chest_model->PrintInfos();

  // _sofa model loading
  _sofa_model = new Model( "../Models/sofa/sofa.dae", 
                            12, 
                            "sofa",
                            _sofa._normal_map,
                            _sofa._height_map );
  _sofa_model->PrintInfos();

  // _sack model loading
  _sack_model = new Model( "../Models/sack/sack.fbx", 
                             13, 
                             "sack",
                             _sack._normal_map,
                             _sack._height_map );
  _sack_model->PrintInfos();

  // _room1_table2 model loading
  _room1_table2_model = new Model( "../Models/room1_table2/room1_table2.dae", 
                             14, 
                             "room1_table2",
                             _room1_table2._normal_map,
                             _room1_table2._height_map );
  _room1_table2_model->PrintInfos();

  // _book model loading
  _book_model = new Model( "../Models/book/book.fbx", 
                           15, 
                           "book",
                           _book._normal_map,
                           _book._height_map );
  _book_model->PrintInfos();

  // _radio model loading
  _radio_model = new Model( "../Models/radio/radio.dae", 
                            16, 
                            "radio",
                            _radio._normal_map,
                            _radio._height_map );
  _radio_model->PrintInfos();*/

  // _screen model loading
  /*_screen_model = new Model( "../Models/screen/screen.dae", 
                            17, 
                            "screen",
                            _screen._normal_map,
                            _screen._height_map );
  _screen_model->PrintInfos();

  // _bike model loading
  _bike_model = new Model( "../Models/bike/bike.fbx", 
                           18, 
                           "bike",
                           _bike._normal_map,
                           _bike._height_map );
  _bike_model->PrintInfos();

  // _pilar model loading
  _pilar_model = new Model( "../Models/pilar/pilar.dae", 
                            19, 
                            "pilar",
                            _pilar._normal_map,
                            _pilar._height_map );
  _pilar_model->PrintInfos();

  // _scanner model loading
  _scanner_model = new Model( "../Models/scanner/scanner.fbx", 
                              20, 
                              "scanner",
                              _scanner._normal_map,
                              _scanner._height_map );
  _scanner_model->PrintInfos();

  // _room2_table1 model loading
  _room2_table1_model = new Model( "../Models/room2_table1/room2_table1.dae", 
                                   21, 
                                   "room2_table1",
                                   _room2_table1._normal_map,
                                   _room2_table1._height_map );
  _room2_table1_model->PrintInfos();

  // _mask model loading
  _mask_model = new Model( "../Models/mask/mask.dae", 
                           22, 
                           "mask",
                           _mask._normal_map,
                           _mask._height_map );
  _mask_model->PrintInfos();

  // _arm model loading
  _arm_model = new Model( "../Models/arm/arm.fbx", 
                          23, 
                          "arm",
                          _arm._normal_map,
                          _arm._height_map );
  _arm_model->PrintInfos();*/

  // _tank model loading
  _tank_model = new Model( "../Models/tank/tank.dae", 
                           24, 
                           "tank",
                           _tank._normal_map,
                           _tank._height_map );
  _tank_model->PrintInfos();

  // _shelving model loading
  _shelving_model = new Model( "../Models/shelving/shelving.fbx", 
                               25, 
                               "shelving",
                               _shelving._normal_map,
                               _shelving._height_map );
  _shelving_model->PrintInfos();

  // _gun1 model loading
  _gun1_model = new Model( "../Models/gun1/gun1.FBX", 
                           26, 
                           "gun1",
                           _gun1._normal_map,
                           _gun1._height_map );
  _gun1_model->PrintInfos();

  // _gun2 model loading
  _gun2_model = new Model( "../Models/gun2/gun2.dae", 
                           27, 
                           "gun2",
                           _gun2._normal_map,
                           _gun2._height_map );
  _gun2_model->PrintInfos();

  // _gun3 model loading
  _gun3_model = new Model( "../Models/gun3/gun3.fbx", 
                           28, 
                           "gun3",
                           _gun3._normal_map,
                           _gun3._height_map );
  _gun3_model->PrintInfos();

  // _room3_table1 model loading
  _room3_table1_model = new Model( "../Models/room3_table1/room3_table1.fbx", 
                                   29, 
                                   "room3_table1",
                                   _room3_table1._normal_map,
                                   _room3_table1._height_map );
  _room3_table1_model->PrintInfos();

  // _room3_table2 model loading
  _room3_table2_model = new Model( "../Models/room3_table2/room3_table2.fbx", 
                                   30, 
                                   "room3_table2",
                                   _room3_table2._normal_map,
                                   _room3_table2._height_map );
  _room3_table2_model->PrintInfos();

  // _helmet model loading
  _helmet_model = new Model( "../Models/helmet/helmet.obj", 
                             31, 
                             "helmet",
                             _helmet._normal_map,
                             _helmet._height_map );
  _helmet_model->PrintInfos();

  std::cout << "Scene's models loading done.\n" << std::endl;
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
  shadow_transform_matrices.push_back( shadow_projection_matrix * glm::lookAt( _lights[ _current_shadow_light_source ]._position, _lights[ _current_shadow_light_source ]._position + glm::vec3( 1.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) ) );
  shadow_transform_matrices.push_back( shadow_projection_matrix * glm::lookAt( _lights[ _current_shadow_light_source ]._position, _lights[ _current_shadow_light_source ]._position + glm::vec3( -1.0f, 0.0f, 0.0f ), glm::vec3 (0.0f, -1.0f, 0.0f ) ) );
  shadow_transform_matrices.push_back( shadow_projection_matrix * glm::lookAt( _lights[ _current_shadow_light_source ]._position, _lights[ _current_shadow_light_source ]._position + glm::vec3( 0.0f, 1.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) ) );
  shadow_transform_matrices.push_back( shadow_projection_matrix * glm::lookAt( _lights[ _current_shadow_light_source ]._position, _lights[ _current_shadow_light_source ]._position + glm::vec3( 0.0f, -1.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, -1.0f ) ) );
  shadow_transform_matrices.push_back( shadow_projection_matrix * glm::lookAt( _lights[ _current_shadow_light_source ]._position, _lights[ _current_shadow_light_source ]._position + glm::vec3( 0.0f, 0.0f, 1.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) ) );
  shadow_transform_matrices.push_back( shadow_projection_matrix * glm::lookAt( _lights[ _current_shadow_light_source ]._position, _lights[ _current_shadow_light_source ]._position + glm::vec3( 0.0f, 0.0f, -1.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) ) );


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
  glUniform3fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uLightPosition" ), 1, &_lights[ _current_shadow_light_source ]._position[ 0 ] );


  // Draw revolving door depth
  // -------------------------
  model_matrix = _revolving_door[ 0 ]._model_matrix;
  glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
  _revolving_door_model->DrawDepth( _point_shadow_depth_shader, model_matrix );


  if( _current_room == 1 )
  {
    // Draw ink bottle depth
    // ---------------------
    model_matrix = _ink_bottle._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _ink_bottle_model->DrawDepth( _point_shadow_depth_shader, model_matrix );


    // Draw room1 table1 depth
    // -----------------------
    model_matrix = _room1_table1._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _room1_table1_model->DrawDepth( _point_shadow_depth_shader, model_matrix );


    // Draw bottle depth
    // -----------------
    model_matrix = _bottle._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _bottle_model->DrawDepth( _point_shadow_depth_shader, model_matrix );


    // Draw ball depth
    // -----------------
    model_matrix = _ball._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _ball_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _box_bag depth
    // -------------------
    model_matrix = _box_bag._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _box_bag_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _chest depth
    // -----------------
    model_matrix = _chest._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _chest_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _sofa depth
    // -----------------
    model_matrix = _sofa._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _sofa_model->DrawDepth( _point_shadow_depth_shader, model_matrix );


    // Draw _sack depth
    // -----------------
    model_matrix = _sack._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _sack_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _room1_table2 depth
    // -----------------------
    model_matrix = _room1_table2._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _room1_table2_model->DrawDepth( _point_shadow_depth_shader, model_matrix );


    // Draw _book depth
    // ----------------
    model_matrix = _book._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _book_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _radio depth
    // ----------------
    model_matrix = _radio._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _radio_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 
  }

  if( _current_room == 2 )
  {

    // Draw _screen depth
    // ------------------
    model_matrix = _screen._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _screen_model->DrawDepth( _point_shadow_depth_shader, model_matrix );


    // Draw _bike depth
    // ----------------
    model_matrix = _bike._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _bike_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _pilar depth
    // -----------------
    model_matrix = _pilar._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _pilar_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _scanner depth
    // -----------------
    model_matrix = _scanner._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _scanner_model->DrawDepth( _point_shadow_depth_shader, model_matrix );


    // Draw _room2_table1 depth
    // ------------------------
    model_matrix = _room2_table1._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _room2_table1_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _mask depth
    // ----------------
    model_matrix = _mask._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _mask_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _arm depth
    // ---------------
    model_matrix = _arm._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _arm_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 
  }

  if( _current_room == 3 )
  {

    // Draw _tank depth
    // ----------------
    model_matrix = _tank._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _tank_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _shelving depth
    // --------------------
    model_matrix = _shelving._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _shelving_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _gun1 depth
    // ----------------
    model_matrix = _gun1._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _gun1_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _gun2 depth
    // ----------------
    model_matrix = _gun2._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _gun2_model->DrawDepth( _point_shadow_depth_shader, model_matrix );


    // Draw _gun3 depth
    // ----------------
    model_matrix = _gun3._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _gun3_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _room3_table1 depth
    // ------------------------
    model_matrix = _room3_table1._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _room3_table1_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _room3_table2 depth
    // ------------------------
    model_matrix = _room3_table2._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _room3_table2_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 


    // Draw _helmet depth
    // ------------------
    model_matrix = _helmet._model_matrix;
    glUniformMatrix4fv( glGetUniformLocation( _point_shadow_depth_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
    _helmet_model->DrawDepth( _point_shadow_depth_shader, model_matrix ); 
  }


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
  //glBindTexture( GL_TEXTURE_CUBE_MAP, _grounds_type1[ _test2 ]._IBL_cubemaps[ 0 ] ); 
  //glBindTexture( GL_TEXTURE_CUBE_MAP, _walls_type1[ _test2 ]._IBL_cubemaps[ 0 ] ); 
  //glBindTexture( GL_TEXTURE_CUBE_MAP, _revolving_door[ _test2 ]._IBL_cubemaps[ _test2 ] ); 
  //glBindTexture( GL_TEXTURE_CUBE_MAP, _room1_table1._IBL_cubemaps[ _test2 ] ); 
  glBindTexture( GL_TEXTURE_CUBE_MAP, _helmet._IBL_cubemaps[ _test2 ] ); 

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
  for( int ground_it = _grounds_start_it; ground_it < _grounds_end_it; ground_it ++ )
  {
    ( _grounds_type1[ ground_it ]._height_map == true ) ? current_shader = &_forward_displacement_pbr_shader : current_shader = &_forward_pbr_shader; 

    current_shader->Use();

    model_matrix = _grounds_type1[ ground_it ]._model_matrix; 
   
    // Textures binding
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _grounds_type1[ ground_it ]._material_id ][ 0 ] );  
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _grounds_type1[ ground_it ]._material_id ][ 1 ] ); 
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _grounds_type1[ ground_it ]._material_id ][ 2 ] ); 
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _grounds_type1[ ground_it ]._material_id ][ 3 ] ); 
    glActiveTexture( GL_TEXTURE4 );
    glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _grounds_type1[ ground_it ]._material_id ][ 4 ] ); 
    glActiveTexture( GL_TEXTURE5 );
    glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _grounds_type1[ ground_it ]._material_id ][ 5 ] ); 
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _grounds_type1[ ground_it ]._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _grounds_type1[ ground_it ]._IBL_cubemaps[ 2 ] ); 
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uIBL" ), _grounds_type1[ ground_it ]._IBL );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uParallaxCubemap" ), _grounds_type1[ ground_it ]._parallax_cubemap );
    glUniform3fv( glGetUniformLocation( current_shader->_program, "uCubemapPos" ), 1, &_grounds_type1[ ground_it ]._IBL_position[ 0 ] );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uIsWall" ), false );

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
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowBias" ), _grounds_type1[ ground_it ]._shadow_bias );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowDarkness" ), _grounds_type1[ ground_it ]._shadow_darkness );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uEmissive" ), _grounds_type1[ ground_it ]._emissive );
    if( _grounds_type1[ ground_it ]._emissive )
    {
      glActiveTexture( GL_TEXTURE11 );
      glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _grounds_type1[ ground_it ]._material_id ][ 6 ] );
      glUniform1f( glGetUniformLocation( current_shader->_program, "uEmissiveFactor" ), _grounds_type1[ ground_it ]._emissive_factor );
    }

    glUniform1f( glGetUniformLocation( current_shader->_program, "uID" ), _grounds_type1[ ground_it ]._id );  

    ( _grounds_type1[ ground_it ]._id == 18 ) ? glBindVertexArray( _ground2_VAO ) : glBindVertexArray( _ground1_VAO );
    ( _grounds_type1[ ground_it ]._height_map == true ) ? glDrawElements( GL_PATCHES, _ground1_indices.size(), GL_UNSIGNED_INT, 0 ) : glDrawElements( GL_TRIANGLES, _ground1_indices.size(), GL_UNSIGNED_INT, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
  }


  // Draw walls type 1
  // -----------------
  for( unsigned int wall_it = _walls_start_it; wall_it < _walls_end_it; wall_it++ )
  {
    ( _walls_type1[ wall_it ]._height_map == true ) ? current_shader = &_forward_displacement_pbr_shader : current_shader = &_forward_pbr_shader; 

    current_shader->Use();

    model_matrix = _walls_type1[ wall_it ]._model_matrix;

    // Textures binding
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _walls_type1[ wall_it ]._material_id ][ 0 ] );  
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _walls_type1[ wall_it ]._material_id ][ 1 ] ); 
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _walls_type1[ wall_it ]._material_id ][ 2 ] ); 
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _walls_type1[ wall_it ]._material_id ][ 3 ] ); 
    glActiveTexture( GL_TEXTURE4 );
    glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _walls_type1[ wall_it ]._material_id ][ 4 ] ); 
    glActiveTexture( GL_TEXTURE5 );
    glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _walls_type1[ wall_it ]._material_id ][ 5 ] ); 
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _walls_type1[ wall_it ]._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _walls_type1[ wall_it ]._IBL_cubemaps[ 2 ] ); 
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uIBL" ), _walls_type1[ wall_it ]._IBL );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uParallaxCubemap" ), _walls_type1[ wall_it ]._parallax_cubemap );
    glUniform3fv( glGetUniformLocation( current_shader->_program, "uCubemapPos" ), 1, &_walls_type1[ wall_it ]._IBL_position[ 0 ] );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uIsWall" ), true );

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
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowBias" ), _walls_type1[ wall_it ]._shadow_bias );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowDarkness" ), _walls_type1[ wall_it ]._shadow_darkness );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uEmissive" ), _walls_type1[ wall_it ]._emissive );
    if( _walls_type1[ wall_it ]._emissive )
    {
      glActiveTexture( GL_TEXTURE11 );
      glBindTexture( GL_TEXTURE_2D, _loaded_materials[ _walls_type1[ wall_it ]._material_id ][ 6 ] );
      glUniform1f( glGetUniformLocation( current_shader->_program, "uEmissiveFactor" ), _walls_type1[ wall_it ]._emissive_factor );
    }
    
    glUniform1f( glGetUniformLocation( current_shader->_program, "uID" ), _walls_type1[ wall_it ]._id );  

    ( _walls_type1[ wall_it ]._id == 4 ) ? glBindVertexArray( _wall2_VAO ) : glBindVertexArray( _wall1_VAO );
    ( _walls_type1[ wall_it ]._height_map == true ) ? glDrawElements( GL_PATCHES, _wall1_indices.size(), GL_UNSIGNED_INT, 0 ) : glDrawElements( GL_TRIANGLES, _wall1_indices.size(), GL_UNSIGNED_INT, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
  }


  // Draw simple doors
  // -----------------
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  _forward_pbr_shader.Use();

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

  // IBL uniforms
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), true );
  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), false );


  for( int door_it = 0; door_it < _simple_door.size(); door_it++ )
  { 
    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _simple_door[ door_it ]._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _simple_door[ door_it ]._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _simple_door[ door_it ]._model_matrix;

    // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _simple_door[ door_it ]._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _simple_door[ door_it ]._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _simple_door[ door_it ]._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _simple_door[ door_it ]._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _simple_door[ door_it ]._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _simple_door[ door_it ]._emissive );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _simple_door[ door_it ]._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _simple_door[ door_it ]._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _simple_door[ door_it ]._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _simple_door[ door_it ]._id );      

    _simple_door_model->Draw( _forward_pbr_shader, model_matrix );
  }

  glUseProgram( 0 );
  glDisable( GL_CULL_FACE );


  // Draw top lights
  // ---------------
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  _forward_pbr_shader.Use();

  for( int light_it = 0; light_it < _top_light.size(); light_it++ )
  { 
    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _top_light[ light_it ]._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _top_light[ light_it ]._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _top_light[ light_it ]._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _top_light[ light_it ]._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _top_light[ light_it ]._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _top_light[ light_it ]._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _top_light[ light_it ]._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _top_light[ light_it ]._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _top_light[ light_it ]._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _top_light[ light_it ]._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _top_light[ light_it ]._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _top_light[ light_it ]._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _top_light[ light_it ]._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _top_light[ light_it ]._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _top_light[ light_it ]._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _top_light[ light_it ]._id );      

    _top_light_model->Draw( _forward_pbr_shader, model_matrix );
  }

  glUseProgram( 0 );
  glDisable( GL_CULL_FACE );


  // Draw wall lights
  // ----------------
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  _forward_pbr_shader.Use();

  for( int light_it = 0; light_it < _wall_light.size(); light_it++ )
  { 
    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _wall_light[ light_it ]._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _wall_light[ light_it ]._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _wall_light[ light_it ]._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _wall_light[ light_it ]._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _wall_light[ light_it ]._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _wall_light[ light_it ]._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _wall_light[ light_it ]._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _wall_light[ light_it ]._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _wall_light[ light_it ]._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _wall_light[ light_it ]._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _wall_light[ light_it ]._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _wall_light[ light_it ]._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _wall_light[ light_it ]._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _wall_light[ light_it ]._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _wall_light[ light_it ]._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _wall_light[ light_it ]._id );      

    _wall_light_model->Draw( _forward_pbr_shader, model_matrix );
  }

  glUseProgram( 0 );
  glDisable( GL_CULL_FACE );


  if( _current_room == 1 )
  {
    // Draw room1 table1
    // -----------------
    _forward_pbr_shader.Use();

    model_matrix = _room1_table1._model_matrix;

    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _room1_table1._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _room1_table1._IBL_cubemaps[ 2 ] ); 
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
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _room1_table1._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _room1_table1._bloom_brightness );

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _room1_table1._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _room1_table1._parallax_cubemap );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _room1_table1._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _room1_table1._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _room1_table1._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _room1_table1._emissive );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _room1_table1._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _room1_table1._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _room1_table1._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _room1_table1._id );      

    _room1_table1_model->Draw( _forward_pbr_shader, model_matrix );

    glUseProgram( 0 );


    // Draw bottle
    // -----------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    _forward_pbr_shader.Use();

    model_matrix = _bottle._model_matrix;

    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _bottle._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _bottle._IBL_cubemaps[ 2 ] ); 
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
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _bottle._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _bottle._bloom_brightness );

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _bottle._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _bottle._parallax_cubemap );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _bottle._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _bottle._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _bottle._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _bottle._emissive );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _bottle._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _bottle._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _bottle._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _bottle._id );      

    _bottle_model->Draw( _forward_pbr_shader, model_matrix );

    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw ball
    // ---------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    ( _ball._height_map == true ) ? current_shader = &_forward_displacement_pbr_shader : current_shader = &_forward_pbr_shader; 

    current_shader->Use();

    model_matrix = _ball._model_matrix;

    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _ball._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _ball._IBL_cubemaps[ 2 ] ); 
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
    glUniform1i( glGetUniformLocation( current_shader->_program, "uBloom" ), _ball._bloom );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uBloomBrightness" ), _ball._bloom_brightness );

    // IBL uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uIBL" ), _ball._IBL );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uParallaxCubemap" ), _ball._parallax_cubemap );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( current_shader->_program, "uAlpha" ), _ball._alpha );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uOpacityMap" ), _ball._opacity_map );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uNormalMap" ), _ball._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uEmissive" ), _ball._emissive );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uReceivShadow" ), _ball._receiv_shadow );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowBias" ), _ball._shadow_bias );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowDarkness" ), _ball._shadow_darkness );

    glUniform1f( glGetUniformLocation( current_shader->_program, "uID" ), _ball._id );      

    _ball_model->Draw( *current_shader, model_matrix );

    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw box bag
    // ------------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    ( _box_bag._height_map == true ) ? current_shader = &_forward_displacement_pbr_shader : current_shader = &_forward_pbr_shader; 

    current_shader->Use();

    model_matrix = _box_bag._model_matrix;

    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _box_bag._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _box_bag._IBL_cubemaps[ 2 ] ); 
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
    glUniform1i( glGetUniformLocation( current_shader->_program, "uBloom" ), _box_bag._bloom );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uBloomBrightness" ), _box_bag._bloom_brightness );

    // IBL uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uIBL" ), _box_bag._IBL );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uParallaxCubemap" ), _box_bag._parallax_cubemap );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( current_shader->_program, "uAlpha" ), _box_bag._alpha );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uOpacityMap" ), _box_bag._opacity_map );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uNormalMap" ), _box_bag._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uEmissive" ), _box_bag._emissive );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uReceivShadow" ), _box_bag._receiv_shadow );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowBias" ), _box_bag._shadow_bias );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowDarkness" ), _box_bag._shadow_darkness );

    glUniform1f( glGetUniformLocation( current_shader->_program, "uID" ), _box_bag._id );      

    _box_bag_model->Draw( *current_shader, model_matrix );

    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw chest
    // ----------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    ( _chest._height_map == true ) ? current_shader = &_forward_displacement_pbr_shader : current_shader = &_forward_pbr_shader; 

    current_shader->Use();

    model_matrix = _chest._model_matrix;

    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _chest._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _chest._IBL_cubemaps[ 2 ] ); 
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
    glUniform1i( glGetUniformLocation( current_shader->_program, "uBloom" ), _chest._bloom );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uBloomBrightness" ), _chest._bloom_brightness );

    // IBL uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uIBL" ), _chest._IBL );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uParallaxCubemap" ), _chest._parallax_cubemap );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( current_shader->_program, "uAlpha" ), _chest._alpha );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uOpacityMap" ), _chest._opacity_map );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uNormalMap" ), _chest._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uEmissive" ), _chest._emissive );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uReceivShadow" ), _chest._receiv_shadow );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowBias" ), _chest._shadow_bias );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowDarkness" ), _chest._shadow_darkness );

    glUniform1f( glGetUniformLocation( current_shader->_program, "uID" ), _chest._id );      

    _chest_model->Draw( *current_shader, model_matrix );

    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw sofa
    // ----------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    ( _sofa._height_map == true ) ? current_shader = &_forward_displacement_pbr_shader : current_shader = &_forward_pbr_shader; 

    current_shader->Use();

    model_matrix = _sofa._model_matrix;

    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _sofa._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _sofa._IBL_cubemaps[ 2 ] ); 
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
    glUniform1i( glGetUniformLocation( current_shader->_program, "uBloom" ), _sofa._bloom );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uBloomBrightness" ), _sofa._bloom_brightness );

    // IBL uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uIBL" ), _sofa._IBL );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uParallaxCubemap" ), _sofa._parallax_cubemap );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( current_shader->_program, "uAlpha" ), _sofa._alpha );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uOpacityMap" ), _sofa._opacity_map );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uNormalMap" ), _sofa._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uEmissive" ), _sofa._emissive );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uReceivShadow" ), _sofa._receiv_shadow );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowBias" ), _sofa._shadow_bias );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowDarkness" ), _sofa._shadow_darkness );

    glUniform1f( glGetUniformLocation( current_shader->_program, "uID" ), _sofa._id );      

    _sofa_model->Draw( *current_shader, model_matrix );

    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw sack
    // ---------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    ( _sack._height_map == true ) ? current_shader = &_forward_displacement_pbr_shader : current_shader = &_forward_pbr_shader; 

    current_shader->Use();

    model_matrix = _sack._model_matrix;

    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _sack._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _sack._IBL_cubemaps[ 2 ] ); 
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
    glUniform1i( glGetUniformLocation( current_shader->_program, "uBloom" ), _sack._bloom );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uBloomBrightness" ), _sack._bloom_brightness );

    // IBL uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uIBL" ), _sack._IBL );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uParallaxCubemap" ), _sack._parallax_cubemap );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( current_shader->_program, "uAlpha" ), _sack._alpha );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uOpacityMap" ), _sack._opacity_map );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uNormalMap" ), _sack._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uEmissive" ), _sack._emissive );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uReceivShadow" ), _sack._receiv_shadow );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowBias" ), _sack._shadow_bias );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowDarkness" ), _sack._shadow_darkness );

    glUniform1f( glGetUniformLocation( current_shader->_program, "uID" ), _sack._id );      

    _sack_model->Draw( *current_shader, model_matrix );

    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw room1_table2
    // -----------------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    ( _room1_table2._height_map == true ) ? current_shader = &_forward_displacement_pbr_shader : current_shader = &_forward_pbr_shader; 

    current_shader->Use();

    model_matrix = _room1_table2._model_matrix;

    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _room1_table2._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _room1_table2._IBL_cubemaps[ 2 ] ); 
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
    glUniform1i( glGetUniformLocation( current_shader->_program, "uBloom" ), _room1_table2._bloom );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uBloomBrightness" ), _room1_table2._bloom_brightness );

    // IBL uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uIBL" ), _room1_table2._IBL );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uParallaxCubemap" ), _room1_table2._parallax_cubemap );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( current_shader->_program, "uAlpha" ), _room1_table2._alpha );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uOpacityMap" ), _room1_table2._opacity_map );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uNormalMap" ), _room1_table2._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uEmissive" ), _room1_table2._emissive );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uReceivShadow" ), _room1_table2._receiv_shadow );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowBias" ), _room1_table2._shadow_bias );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowDarkness" ), _room1_table2._shadow_darkness );

    glUniform1f( glGetUniformLocation( current_shader->_program, "uID" ), _room1_table2._id );      

    _room1_table2_model->Draw( *current_shader, model_matrix );

    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw book
    // ---------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    ( _book._height_map == true ) ? current_shader = &_forward_displacement_pbr_shader : current_shader = &_forward_pbr_shader; 

    current_shader->Use();

    model_matrix = _book._model_matrix;

    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _book._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _book._IBL_cubemaps[ 2 ] ); 
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
    glUniform1i( glGetUniformLocation( current_shader->_program, "uBloom" ), _book._bloom );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uBloomBrightness" ), _book._bloom_brightness );

    // IBL uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uIBL" ), _book._IBL );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uParallaxCubemap" ), _book._parallax_cubemap );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( current_shader->_program, "uAlpha" ), _book._alpha );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uOpacityMap" ), _book._opacity_map );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uNormalMap" ), _book._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uEmissive" ), _book._emissive );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uReceivShadow" ), _book._receiv_shadow );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowBias" ), _book._shadow_bias );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowDarkness" ), _book._shadow_darkness );

    glUniform1f( glGetUniformLocation( current_shader->_program, "uID" ), _book._id );      

    _book_model->Draw( *current_shader, model_matrix );

    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw radio
    // ----------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    ( _radio._height_map == true ) ? current_shader = &_forward_displacement_pbr_shader : current_shader = &_forward_pbr_shader; 

    current_shader->Use();

    model_matrix = _radio._model_matrix;

    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _radio._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _radio._IBL_cubemaps[ 2 ] ); 
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
    glUniform1i( glGetUniformLocation( current_shader->_program, "uBloom" ), _radio._bloom );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uBloomBrightness" ), _radio._bloom_brightness );

    // IBL uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uIBL" ), _radio._IBL );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uParallaxCubemap" ), _radio._parallax_cubemap );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( current_shader->_program, "uAlpha" ), _radio._alpha );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uOpacityMap" ), _radio._opacity_map );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uNormalMap" ), _radio._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uEmissive" ), _radio._emissive );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( current_shader->_program, "uReceivShadow" ), _radio._receiv_shadow );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( current_shader->_program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowBias" ), _radio._shadow_bias );
    glUniform1f( glGetUniformLocation( current_shader->_program, "uShadowDarkness" ), _radio._shadow_darkness );

    glUniform1f( glGetUniformLocation( current_shader->_program, "uID" ), _radio._id );      

    _radio_model->Draw( *current_shader, model_matrix );

    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw ink bottle
    // ---------------
    _forward_pbr_shader.Use();

    model_matrix = _ink_bottle._model_matrix;

    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _ink_bottle._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _ink_bottle._IBL_cubemaps[ 2 ] ); 
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), true );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), false );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _ink_bottle._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _ink_bottle._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _ink_bottle._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _ink_bottle._emissive );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _ink_bottle._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _ink_bottle._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _ink_bottle._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _ink_bottle._id );      

    _ink_bottle_model->Draw( _forward_pbr_shader, model_matrix );

    glUseProgram( 0 );
  }

  if( _current_room == 2 )
  {

    // Draw screen
    // -----------
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _screen._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _screen._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _screen._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _screen._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _screen._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _screen._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _screen._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _screen._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _screen._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _screen._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _screen._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _screen._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _screen._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _screen._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _screen._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _screen._id );      

    _screen_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );


    // Draw bike
    // ---------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _bike._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _bike._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _bike._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _bike._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _bike._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _bike._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _bike._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _bike._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _bike._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _bike._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _bike._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _bike._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _bike._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _bike._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _bike._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _bike._id );      

    _bike_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw pilar
    // ----------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _pilar._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _pilar._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _pilar._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _pilar._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _pilar._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _pilar._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _pilar._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _pilar._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _pilar._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _pilar._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _pilar._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _pilar._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _pilar._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _pilar._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _pilar._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _pilar._id );      

    _pilar_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw scanner
    // ------------
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _scanner._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _scanner._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _scanner._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _scanner._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _scanner._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _scanner._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _scanner._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _scanner._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _scanner._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _scanner._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _scanner._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _scanner._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _scanner._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _scanner._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _scanner._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _scanner._id );      

    _scanner_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );


    // Draw room2_table1
    // -----------------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _room2_table1._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _room2_table1._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _room2_table1._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _room2_table1._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _room2_table1._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _room2_table1._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _room2_table1._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _room2_table1._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _room2_table1._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _room2_table1._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _room2_table1._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _room2_table1._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _room2_table1._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _room2_table1._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _room2_table1._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _room2_table1._id );      

    _room2_table1_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw mask
    // ---------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _mask._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _mask._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _mask._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _mask._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _mask._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _mask._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _mask._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _mask._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _mask._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _mask._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _mask._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _mask._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _mask._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _mask._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _mask._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _mask._id );      

    _mask_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw arm
    // --------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _arm._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _arm._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _arm._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _arm._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _arm._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _arm._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _arm._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _arm._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _arm._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _arm._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _arm._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _arm._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _arm._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _arm._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _arm._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _arm._id );      

    _arm_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );

  }

  if( _current_room == 3 )
  {

    // Draw tank
    // ---------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _tank._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _tank._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _tank._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _tank._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _tank._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _tank._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _tank._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _tank._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _tank._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _tank._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _tank._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _tank._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _tank._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _tank._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _tank._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _tank._id );      

    _tank_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw shelving
    // -------------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _shelving._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _shelving._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _shelving._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _shelving._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _shelving._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _shelving._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _shelving._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _shelving._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _shelving._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _shelving._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _shelving._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _shelving._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _shelving._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _shelving._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _shelving._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _shelving._id );      

    _shelving_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw gun1
    // ---------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _gun1._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _gun1._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _gun1._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _gun1._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _gun1._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _gun1._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _gun1._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _gun1._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _gun1._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _gun1._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _gun1._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _gun1._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _gun1._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _gun1._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _gun1._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _gun1._id );      

    _gun1_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw gun2
    // ---------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _gun2._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _gun2._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _gun2._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _gun2._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _gun2._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _gun2._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _gun2._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _gun2._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _gun2._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _gun2._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _gun2._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _gun2._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _gun2._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _gun2._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _gun2._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _gun2._id );      

    _gun2_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw gun3
    // ---------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _gun3._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _gun3._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _gun3._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _gun3._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _gun3._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _gun3._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _gun3._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _gun3._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _gun3._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _gun3._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _gun3._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _gun3._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _gun3._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _gun3._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _gun3._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _gun3._id );      

    _gun3_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw room3_table1
    // -----------------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _room3_table1._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _room3_table1._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _room3_table1._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _room3_table1._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _room3_table1._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _room3_table1._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _room3_table1._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _room3_table1._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _room3_table1._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _room3_table1._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _room3_table1._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _room3_table1._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _room3_table1._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _room3_table1._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _room3_table1._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _room3_table1._id );      

    _room3_table1_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw room3_table2
    // -----------------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _room3_table2._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _room3_table2._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _room3_table2._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _room3_table2._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _room3_table2._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _room3_table2._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _room3_table2._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _room3_table2._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _room3_table2._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _room3_table2._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _room3_table2._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _room3_table2._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _room3_table2._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _room3_table2._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _room3_table2._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _room3_table2._id );      

    _room3_table2_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );


    // Draw helmet
    // -----------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    _forward_pbr_shader.Use();

    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _helmet._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _helmet._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

    model_matrix = _helmet._model_matrix;

     // Matrices uniforms
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_view_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( _camera->_projection_matrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _forward_pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( model_matrix ) );
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

    // IBL uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), _helmet._IBL );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), _helmet._parallax_cubemap );

    // Bloom uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uBloom" ), _helmet._bloom );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uBloomBrightness" ), _helmet._bloom_brightness );

    // Opacity uniforms
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uAlpha" ), _helmet._alpha );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityMap" ), _helmet._opacity_map );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uOpacityDiscard" ), 1.0 );
    
    // Displacement mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uNormalMap" ), _helmet._normal_map );

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _helmet._emissive );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uEmissiveFactor" ), _helmet._emissive_factor );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _helmet._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowBias" ), _helmet._shadow_bias );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowDarkness" ), _helmet._shadow_darkness );

    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uID" ), _helmet._id );      

    _helmet_model->Draw( _forward_pbr_shader, model_matrix );
    glUseProgram( 0 );
    glDisable( GL_CULL_FACE );
  }


  // Draw revolving doors
  // --------------------
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  _forward_pbr_shader.Use();

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

  // IBL uniforms
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uIBL" ), true );
  glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uMaxMipLevel" ), ( float )( _pre_filter_max_mip_Level - 1 ) );
  glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uParallaxCubemap" ), false );


  for( int door_it = 0; door_it < _revolving_door.size(); door_it++ )
  { 
    // IBL cubemap texture binding
    glActiveTexture( GL_TEXTURE7 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _revolving_door[ door_it ]._IBL_cubemaps[ 1 ] );
    glActiveTexture( GL_TEXTURE8 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _revolving_door[ door_it ]._IBL_cubemaps[ 2 ] ); 
    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_2D, _pre_brdf_texture ); 
    glActiveTexture( GL_TEXTURE10 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _window->_toolbox->_depth_cubemap );

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

    // Emissive uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uEmissive" ), _revolving_door[ door_it ]._emissive );

    // Omnidirectional shadow mapping uniforms
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uReceivShadow" ), _revolving_door[ door_it ]._receiv_shadow );
    glUniform1f( glGetUniformLocation( _forward_pbr_shader._program, "uShadowFar" ), _shadow_far );
    glUniform1i( glGetUniformLocation( _forward_pbr_shader._program, "uLightSourceIt" ), _current_shadow_light_source );
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
  glBindTexture( GL_TEXTURE_2D, _loaded_materials[ 0 ][ 0 ] );  
  glActiveTexture( GL_TEXTURE1 );
  glBindTexture( GL_TEXTURE_2D, _loaded_materials[ 0 ][ 1 ] ); 
  glActiveTexture( GL_TEXTURE3 );
  glBindTexture( GL_TEXTURE_2D, _loaded_materials[ 0 ][ 3 ] ); 
  glActiveTexture( GL_TEXTURE4 );
  glBindTexture( GL_TEXTURE_2D, _loaded_materials[ 0 ][ 4 ] ); 
  glActiveTexture( GL_TEXTURE5 );
  glBindTexture( GL_TEXTURE_2D, _loaded_materials[ 0 ][ 5 ] ); 

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

  switch( _current_room ) 
  {
    case 1:
      _walls_start_it = 0;
      _walls_end_it   = 23;
    
      _grounds_start_it = 0;
      _grounds_end_it   = 2;  
      break;

    case 2:
      _walls_start_it = 23;
      _walls_end_it   = 49;
    
      _grounds_start_it = 2;
      _grounds_end_it   = 4;
      break;

    case 3:
      _walls_start_it = 49;
      _walls_end_it   = _walls_type1.size();
    
      _grounds_start_it = 4;
      _grounds_end_it   = _grounds_type1.size();
      break;

    default:
      break;
  }  


  // Revolving door rotation matrix update
  // -------------------------------------
  _door_rotation_matrix = glm::mat4();
  _door_rotation_matrix = glm::rotate( _door_rotation_matrix, _clock->GetCurrentTime() * 0.25f, glm::vec3( 0.0, 0.0, 1.0 ) );

  _door1_rotation_matrix = glm::mat4();
  _door1_rotation_matrix = glm::rotate( _door1_rotation_matrix, _door_angle, glm::vec3( 0.0, 0.0, -1.0 ) );

  _door2_rotation_matrix = glm::mat4();
  _door2_rotation_matrix = glm::rotate( _door2_rotation_matrix, _door_angle, glm::vec3( 0.0, 0.0, 1.0 ) );


  // Simple door translation matrix update
  // -------------------------------------
  _door_translation_matrix1 = glm::mat4();
  _door_translation_matrix1 = glm::translate( _door_translation_matrix1, _door_position );

  _door_translation_matrix2 = glm::mat4();
  _door_translation_matrix2 = glm::translate( _door_translation_matrix2, -_door_position );
  _door_translation_matrix2 = glm::rotate( _door_translation_matrix2, ( float )_PI, glm::vec3( 0.0, 1.0, 0.0 ) );

  RevolvingDoorScript();
  SimpleDoorScript();
}

void Scene::RevolvingDoorScript()
{
  if( _revolving_door_open && _door_angle < 0.53 )
  {
    _door_angle += 0.1 * _clock->GetDeltaTime();
  }

  if( !_revolving_door_open )
  {
    _door_angle = 0.0;
  }
}

void Scene::SimpleDoorScript()
{
  if( _simple_door_open && _door_position.x < 0.5 )
  {
    _door_position.x += 0.1 * _clock->GetDeltaTime();
  }

  if( !_simple_door_open )
  {
    _door_position.x = 0.0;
  }
}

void Scene::ObjectCubemapsGeneration( Object *     iObject,
                                      bool         iNeedAllWalls,
                                      unsigned int iWallID )
{
  iObject->_IBL_cubemaps.push_back( _window->_toolbox->GenEnvironmentCubemap( iObject->_IBL_position,
                                                                              iObject->_id,
                                                                              iNeedAllWalls,
                                                                              iWallID ) );

  iObject->_IBL_cubemaps.push_back( _window->_toolbox->GenIrradianceCubeMap( iObject->_IBL_cubemaps[ 0 ],
                                                                            _res_irradiance_cubemap,
                                                                            _diffuse_irradiance_shader,
                                                                            _irradiance_sample_delta ) );

  iObject->_IBL_cubemaps.push_back( _window->_toolbox->GenPreFilterCubeMap( iObject->_IBL_cubemaps[ 0 ],
                                                                           _res_pre_filter_cubemap,
                                                                           _specular_pre_filter_shader,
                                                                           _pre_filter_sample_count,
                                                                           _pre_filter_max_mip_Level ) );
}

void Scene::ObjectsIBLInitialization()
{
  AnimationsUpdate();


  // Scene's objects environment generation
  //---------------------------------------
  
  std::cout << "Scene's objects environment generation in progress..." << std::endl;


  for( int i = 0; i < _revolving_door.size(); i++ )
  {
    ObjectCubemapsGeneration( &_revolving_door[ i ],
                              true,
                              0 );
  }

  for( int i = 0; i < _simple_door.size(); i++ )
  {
    ObjectCubemapsGeneration( &_simple_door[ i ],
                              true,
                              0 );
  }

  for( int i = 0; i < _top_light.size(); i++ )
  {
    ObjectCubemapsGeneration( &_top_light[ i ],
                              true,
                              0 );
  }

  for( int i = 0; i < _wall_light.size(); i++ )
  {
    ObjectCubemapsGeneration( &_wall_light[ i ],
                              true,
                              0 );
  }

  for( int i = 0; i < _walls_type1.size(); i++ )
  {
    ObjectCubemapsGeneration( &_walls_type1[ i ],
                              true,
                              0 );
  }

  for( int i = 0; i < _grounds_type1.size(); i++ )
  {
    ObjectCubemapsGeneration( &_grounds_type1[ i ],
                              true,
                              0 );
  }
  
  ObjectCubemapsGeneration( &_ink_bottle,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_room1_table1,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_bottle,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_ball,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_box_bag,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_chest,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_sofa,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_sack,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_room1_table2,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_book,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_radio,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_screen,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_bike,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_pilar,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_scanner,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_room2_table1,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_mask,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_arm,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_tank,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_shelving,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_gun1,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_gun2,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_gun3,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_room3_table1,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_room3_table2,
                            true,
                            0 );

  ObjectCubemapsGeneration( &_helmet,
                            true,
                            0 );

  std::cout << "Scene's objects environment generation done.\n" << std::endl;
}