#include "scene.hpp"
#include "window.hpp"


//******************************************************************************
//**********  Class Scene  *****************************************************
//******************************************************************************

Scene::Scene( Window * iParentWindow )
{ 
  _skyboxVAO   = 0;
  _lampVAO     = 0;
  _groundVAO   = 0;

  _skyboxVBO   = 0;
  _lampVBO     = 0;
  _groundVBO   = 0;

  _tex_albedo_ground    = 0;
  _tex_normal_ground    = 0;
  _tex_height_ground    = 0;
  _tex_AO_ground        = 0;
  _tex_roughness_ground = 0;
  _tex_metalness_ground = 0;

  // Init bloom param
  _exposure         = 0.65;
  _bloom            = true;
  _bloom_downsample = 0.5;

  // Init multi sample param
  _multi_sample    = false;
  _nb_multi_sample = 4;

  // Init IBL param
  _res_IBL_cubeMap        = 512;
  _res_irradiance_cubeMap = 32;

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
  LoadModels();

  // Create lights
  _lights.clear();
  LightsInitialization();  

  // Init scene data 
  SceneDataInitialization();
}

void Scene::Quit()
{

  // Delete textures
  // ---------------
  if( _window->_toolbox->_pingpongColorbuffers[ 0 ] )
    glDeleteTextures( 1, &_window->_toolbox->_pingpongColorbuffers[ 0 ] );
  if( _window->_toolbox->_pingpongColorbuffers[ 1 ] )
    glDeleteTextures( 1, &_window->_toolbox->_pingpongColorbuffers[ 1 ] );
  if( _hdrTexture )
    glDeleteTextures( 1, &_hdrTexture );
  if( _envCubemap )
    glDeleteTextures( 1, &_envCubemap );
  if( _irradianceMap )
    glDeleteTextures( 1, &_irradianceMap );
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
  if( _skyboxVAO )
    glDeleteVertexArrays( 1, &_skyboxVAO );
  if( _lampVAO )
    glDeleteVertexArrays( 1, &_lampVAO );
  if( _groundVAO )
    glDeleteVertexArrays( 1, &_groundVAO );
  

  // Delete VBOs
  // -----------
  if( _lampVBO )
    glDeleteBuffers( 1, &_lampVBO );
  if( _skyboxVBO )
    glDeleteBuffers( 1, &_skyboxVBO );
  if( _groundVBO )
    glDeleteBuffers( 1, &_groundVBO );


  // Delete FBOs
  // -----------
  if( _window->_toolbox->_hdrFBO )
    glDeleteFramebuffers( 1, &_window->_toolbox->_hdrFBO );
  if( _window->_toolbox->_final_hdr_FBO )
    glDeleteFramebuffers( 1, &_window->_toolbox->_final_hdr_FBO );
  if( _window->_toolbox->_hdrFBO )
    glDeleteFramebuffers( 1, &_window->_toolbox->_hdrFBO );
  if( _window->_toolbox->_captureFBO )
    glDeleteFramebuffers( 1, &_window->_toolbox->_captureFBO );


  // Delete RBOs
  // -----------
  if( _window->_toolbox->_dephtRBO )
    glDeleteRenderbuffers( 1, &_window->_toolbox->_dephtRBO );
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

  // Create cube geometry
  GLfloat skyboxVertices[] =
  {
    // Back Face
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
     0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
     0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
     0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
    -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left

    // Front face
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
     0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
     0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
     0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
    -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left

    // Left face
    -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
    -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
    -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right

    // Right face
    0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
    0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
    0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
    0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     

    // Bottom face
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
     0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
     0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
     0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
    -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right

    // Top face
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
     0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
     0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right     
     0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
    -0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f  // bottom-left        
  };

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


  // Create skybox VAO
  // -----------------
  glGenVertexArrays( 1, &_skyboxVAO );
  glBindVertexArray( _skyboxVAO );
  glGenBuffers( 1, &_skyboxVBO );
  glBindBuffer( GL_ARRAY_BUFFER, _skyboxVBO );
  glBufferData( GL_ARRAY_BUFFER, sizeof( skyboxVertices ), &skyboxVertices, GL_STATIC_DRAW );
  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( GLfloat ), ( GLvoid* )0 );
  glEnableVertexAttribArray( 1 );
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( GLfloat ), ( GLvoid* )( 3 * sizeof( GLfloat ) ) );
  glEnableVertexAttribArray( 2 );
  glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof( GLfloat ), ( GLvoid* )( 6 * sizeof( GLfloat ) ) );
  glBindVertexArray( 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );

  // Skybox texture
  _faces.push_back( "../skybox/s1/front.png" );
  _faces.push_back( "../skybox/s1/back.png" );
  _faces.push_back( "../skybox/s1/top.png" );
  _faces.push_back( "../skybox/s1/bottom.png" );
  _faces.push_back( "../skybox/s1/right.png" );
  _faces.push_back( "../skybox/s1/left.png" );


  // IBL all pre process
  // -------------------

  // Load hdr skybox texture
  _window->_toolbox->_hdr_image_manager->stbi_set_flip_vertically_on_load( true );
  int width, height, nrComponents;
  //std::string temp_str("../skybox/hdr skybox 2/Ridgecrest_Road_Ref.hdr");
  std::string temp_str("../skybox/hdr skybox 1/Arches_E_PineTree_3k.hdr");
  //std::string temp_str("../skybox/hdr skybox 3/QueenMary_Chimney_Ref.hdr");
  float * data = _window->_toolbox->_hdr_image_manager->stbi_loadf( temp_str.c_str(), &width, &height, &nrComponents, 0 );
  if( data )
  {
    glGenTextures( 1, &_hdrTexture );
    glBindTexture( GL_TEXTURE_2D, _hdrTexture );
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
  glGenTextures( 1, &_envCubemap );
  glBindTexture( GL_TEXTURE_CUBE_MAP, _envCubemap );
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
  glBindTexture( GL_TEXTURE_2D, _hdrTexture );
  glUniformMatrix4fv( glGetUniformLocation( _cube_map_converter_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( captureProjection ) );

  glViewport( 0, 0, _res_IBL_cubeMap, _res_IBL_cubeMap ); // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_captureFBO );
  for( unsigned int i = 0; i < 6; ++i )
  {
    glUniformMatrix4fv( glGetUniformLocation( _cube_map_converter_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( captureViews[ i ] ) );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, _envCubemap, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    _window->_toolbox->RenderCube();
  }
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );

  // Gen irradiance cube map texture
  glGenTextures( 1, &_irradianceMap );
  glBindTexture( GL_TEXTURE_CUBE_MAP, _irradianceMap );
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
  glUniform1i( glGetUniformLocation( _diffuse_irradiance_shader._program, "uEnvironmentMap" ), 0 );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, _envCubemap );
  glUniformMatrix4fv( glGetUniformLocation( _diffuse_irradiance_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( captureProjection ) );

  glViewport( 0, 0, _res_irradiance_cubeMap, _res_irradiance_cubeMap ); // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_captureFBO );
  for( unsigned int i = 0; i < 6; ++i )
  {
    glUniformMatrix4fv( glGetUniformLocation( _diffuse_irradiance_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( captureViews[ i ] ) );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, _irradianceMap, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    _window->_toolbox->RenderCube();
  }
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );


  // Create observer VAO
  // -------------------
  glGenVertexArrays( 1, &_window->_toolbox->_observerVAO );
  glBindVertexArray( _window->_toolbox->_observerVAO );
  glEnableVertexAttribArray( 0 );
  glEnableVertexAttribArray( 1 );
  glGenBuffers( 1, &_window->_toolbox->_observerVBO );
  glBindBuffer( GL_ARRAY_BUFFER, _window->_toolbox->_observerVBO );
  glBufferData( GL_ARRAY_BUFFER, sizeof observer, observer, GL_STATIC_DRAW );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0 ,( const void * )( 0*( sizeof( float ) ) ) );  
  glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0 ,( const void * )( 12*( sizeof( float ) ) ) );
  glBindBuffer( GL_ARRAY_BUFFER, 0);
  glBindVertexArray( 0);


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
    // Positions
    glm::vec3 pos1( -1.0,  1.0, 0.0 );
    glm::vec3 pos2( -1.0, -1.0, 0.0 );
    glm::vec3 pos3(  1.0, -1.0, 0.0 );
    glm::vec3 pos4(  1.0,  1.0, 0.0 );
    
    // texture coordinates
    glm::vec2 uv1( 0.0, 1.0 );
    glm::vec2 uv2( 0.0, 0.0 );
    glm::vec2 uv3( 1.0, 0.0 );
    glm::vec2 uv4( 1.0, 1.0 );
    
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
  glGenFramebuffers( 1, &_window->_toolbox->_hdrFBO );
  glGenRenderbuffers( 1, &_window->_toolbox->_dephtRBO );
  glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_hdrFBO );
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
    glBindRenderbuffer( GL_RENDERBUFFER, _window->_toolbox->_dephtRBO );
    glRenderbufferStorageMultisample( GL_RENDERBUFFER, _nb_multi_sample, GL_DEPTH24_STENCIL8, _window->_width, _window->_height ); 
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _window->_toolbox->_dephtRBO );

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
    glBindRenderbuffer( GL_RENDERBUFFER, _window->_toolbox->_dephtRBO );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _window->_width, _window->_height );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _window->_toolbox->_dephtRBO );
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
    

  // Create pingpong buffer
  // ----------------------
  glGenFramebuffers( 2, _window->_toolbox->_pingpongFBO );
  glGenTextures( 2, _window->_toolbox->_pingpongColorbuffers );
  for( GLuint i = 0; i < 2; i++ )
  {
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_pingpongFBO[ i ] );
    glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_pingpongColorbuffers[ i ] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, _window->_width * _bloom_downsample, _window->_height * _bloom_downsample, 0, GL_RGB, GL_FLOAT, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _window->_toolbox->_pingpongColorbuffers[ i ], 0 );
    if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
      std::cout << "Framebuffer not complete!" << std::endl;
    }
  }
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glBindTexture( GL_TEXTURE_2D, 0 );
    

  // Load ground albedo texture
  // ---------------------------- 
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
  _lights.push_back( Light ( glm::vec3( -7, 12, -10 ),
                             glm::vec3( 1.0, 1.0, 1.0 ) * glm::vec3( 6.0 ),
                             1.0 ) );    

 /* _lights.push_back( Light ( glm::vec3( -7, 12, 10 )
                             glm::vec3( 1.0, 1.0, 1.0 ) * glm::vec3( 6.0 ),
                             1.0 ) );

  _lights.push_back( Light ( glm::vec3( 7, 12, -10 ),
                             glm::vec3( 1.0, 1.0, 1.0 ) * glm::vec3( 6.0 ),
                             1.0 ) );

  _lights.push_back( Light ( glm::vec3( 7, 12, -10 ),
                             glm::vec3( 1.0, 1.0, 1.0 ) * glm::vec3( 6.0 ),
                             1.0 ) );*/
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
        position = glm::vec3( -0.8, 0.188, 0.0 );
        break;
      
      case 1:
        position = glm::vec3( 0.0, 0.188, 0.0 );
        break;

      case 2:
        position = glm::vec3( 0.8, 0.188, 0.0 );
        break;
    }

    _tables.push_back( Object( 0,
                               position,
                               0.0,
                               0.0,
                               0.0075,
                               1.0,
                               1.0,
                               1.0,
                               1.0,
                               8,
                               false,
                               0.75,
                               true,   // normal mapping
                               false,   // bloom
                               1.0 ) );
  }


  // _Ground1 object initialization
  // -----------------------------
  _ground1 = new Object( 1,
                         glm::vec3( 0.0, 0.0, 0.0 ),
                         _PI_2,
                         0.0,
                         1.0,
                         1.0,
                         1.0,
                         1.0,
                         1.0,
                         8,
                         false,
                         0.75,
                         true,
                         false,
                         1.0 );

}

void Scene::ShadersInitialization()
{
  // Compilation des shaders
  // -----------------------
  _pbr_shader.SetShaderClassicPipeline(                "../shaders/pbr_lighting.vs",       "../shaders/pbr_lighting.fs" );
  _skybox_shader.SetShaderClassicPipeline(             "../shaders/skybox.vs",             "../shaders/skybox.fs" );
  _flat_color_shader.SetShaderClassicPipeline(         "../shaders/flat_color.vs",         "../shaders/flat_color.fs" );
  _observer_shader.SetShaderClassicPipeline(           "../shaders/observer.vs",           "../shaders/observer.fs" );
  _blur_shader.SetShaderClassicPipeline(               "../shaders/blur.vs",               "../shaders/blur.fs" );
  _bloom_shader.SetShaderClassicPipeline(              "../shaders/bloom_blending.vs",     "../shaders/bloom_blending.fs" );
  _blit_shader.SetShaderClassicPipeline(               "../shaders/blit.vs",               "../shaders/blit.fs" );
  _cube_map_converter_shader.SetShaderClassicPipeline( "../shaders/cube_map_converter.vs", "../shaders/cube_map_converter.fs" );
  _diffuse_irradiance_shader.SetShaderClassicPipeline( "../shaders/cube_map_converter.vs", "../shaders/diffuse_irradiance.fs" );


  // Set texture uniform location
  // ----------------------------
  _pbr_shader.Use();
  glUniform1i( glGetUniformLocation( _pbr_shader._program, "uTextureDiffuse1" ), 0 ) ;
  glUniform1i( glGetUniformLocation( _pbr_shader._program, "uTextureNormal1" ), 1 );
  glUniform1i( glGetUniformLocation( _pbr_shader._program, "uTextureHeight1" ), 2 ) ;
  glUniform1i( glGetUniformLocation( _pbr_shader._program, "uTextureAO1" ), 3 );
  glUniform1i( glGetUniformLocation( _pbr_shader._program, "uTextureRoughness1" ), 4 );
  glUniform1i( glGetUniformLocation( _pbr_shader._program, "uTextureMetalness1" ), 5 );
  glUniform1i( glGetUniformLocation( _pbr_shader._program, "uTextureSpecular1" ), 6 );

  glUniform1i( glGetUniformLocation( _pbr_shader._program, "uIrradianceCubeMap" ), 9 ); 
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

  _bloom_shader.Use();
  glUniform1i( glGetUniformLocation( _bloom_shader._program, "uBaseColorTexture" ), 0 );
  glUniform1i( glGetUniformLocation( _bloom_shader._program, "uBloomBrightnessTexture" ), 1 );
  glUseProgram( 0 );
}

void Scene::LoadModels()
{
  // Table model loading
  _table_model = new Model();
  _table_model->Load_Model( "../Models/cube/Rounded Cube.fbx", 0 );
  _table_model->Print_info_model();
}

void Scene::RenderScene( bool iIsFinalFBO )
{

  // Matrices setting
  // ----------------
  glm::mat4 projectionM, Msend, viewMatrix, Msend2, projectionM2, projectionM3;

  projectionM = glm::perspective( 45.0f, ( float )_window->_width / ( float )_window->_height, _camera->_near, _camera->_far );
  projectionM3 = glm::perspective( 45.0f, ( float )_window->_width / ( float )_window->_height, -_camera->_near, -_camera->_far );
  viewMatrix = glm::lookAt( _camera->_position, _camera->_position + _camera->_front, _camera->_up ); 

  glm::mat4 lightProjection, lightView, light_space_matrix, skybox_light_space_matrix;
  //GLfloat far =  glm::distance(lights[2].lightPos, glm::vec3(house.x,house.y,house.z)) * 1.2f;
  //lightProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 1.0f, far);
  //lightView = glm::lookAt(lights[2].lightPos, glm::vec3(house.x,house.y,house.z) , glm::vec3(0.0,1.0,0.0));
  //light_space_matrix = lightProjection * lightView;


  // Bind correct buffer for drawing
  // -------------------------------
  if( iIsFinalFBO )
  {
    if( _multi_sample )
    {
      glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_hdrFBO /*final_hdr_FBO*/ );
    }
    else{
      glBindFramebuffer( GL_FRAMEBUFFER, /*final_hdr_FBO*/ _window->_toolbox->_hdrFBO );
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
  glBindTexture( GL_TEXTURE_CUBE_MAP, /*irradianceMap*/ _envCubemap ); // bind les 6 textures du cube map 

  glBindVertexArray( _skyboxVAO );
  glEnable( GL_BLEND );
  glDrawArrays( GL_TRIANGLES, 0, 36 );
  glDisable( GL_BLEND );
  glBindVertexArray( 0 );

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
    Msend = glm::scale( Msend, glm::vec3( 1.0f ) ); 
    glm::vec3 lampColor = _lights[ i ]._color;
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


  // Draw _ground1
  // -------------
  _pbr_shader.Use();

  Msend = glm::mat4();

  Msend = glm::translate( Msend, _ground1->_position );
  Msend = glm::rotate( Msend, _ground1->_angle, glm::vec3( -1.0, 0.0 , 0.0 ) );
  Msend = glm::scale( Msend, glm::vec3( _ground1->_scale * 2.0f, _ground1->_scale * 2.0f, _ground1->_scale * 1.0f ) ); 

  projectionM2[ 0 ] = glm::vec4( ( float )( _window->_width / 2.0 ), 0.0, 0.0, ( float )( _window->_width / 2.0 ) );
  projectionM2[ 1 ] = glm::vec4( 0.0, ( float )( _window->_height / 2.0 ), 0.0, ( float )( _window->_height / 2.0 ) );
  projectionM2[ 2 ] = glm::vec4( 0.0, 0.0, 1.0, 0.0 );
  projectionM2[ 3 ] = glm::vec4( 0.0, 0.0, 0.0, 1.0 );

  projectionM2 = glm::transpose( projectionM2 );

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
  glBindTexture( GL_TEXTURE_CUBE_MAP, _irradianceMap /*envCubemap*/ ); // bind les 6 textures du cube map 

  glUniformMatrix4fv( glGetUniformLocation( _pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( viewMatrix ) );
  glUniformMatrix4fv( glGetUniformLocation( _pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( Msend ) );
  glUniformMatrix4fv( glGetUniformLocation( _pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( projectionM ) );

  //glUniformMatrix4fv(glGetUniformLocation(pbr_shader.Program, "uLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));

  glUniform1i( glGetUniformLocation( _pbr_shader._program, "uLightCount" ), _lights.size() );

  for( int i = 0; i < _lights.size(); i++ )
  {
    string temp = to_string( i );
    glUniform3fv( glGetUniformLocation( _pbr_shader._program, ( "uLightPos["+ temp +"]" ).c_str() ),1, &_lights[ i ]._position[ 0 ] );
    glUniform3fv( glGetUniformLocation( _pbr_shader._program, ( "uLightColor["+temp+"]" ).c_str() ),1, &_lights[ i ]._color[ 0 ] );
  }

  glUniform1f( glGetUniformLocation( _pbr_shader._program, "uAmbientSTR" ), _ground1->_ambient_str );
  glUniform1f( glGetUniformLocation( _pbr_shader._program, "uDiffuseSTR" ), _ground1->_diffuse_str );
  glUniform1f( glGetUniformLocation( _pbr_shader._program, "uSpecularSTR" ), _ground1->_specular_str );
  glUniform1f( glGetUniformLocation( _pbr_shader._program, "uShiniSTR" ), _ground1->_shini_str );

  glUniform1i( glGetUniformLocation( _pbr_shader._program, "uBloom" ), _ground1->_bloom );
  glUniform1f( glGetUniformLocation( _pbr_shader._program, "uBloomBrightness" ), _ground1->_bloom_brightness );

  glUniform3fv( glGetUniformLocation( _pbr_shader._program, "uViewPos" ), 1, &_camera->_position[ 0 ] );

  glUniform1f( glGetUniformLocation( _pbr_shader._program, "uAlpha" ), _ground1->_alpha );
  glUniform1f( glGetUniformLocation( _pbr_shader._program, "uID" ), _ground1->_id );    
  glUniform1f( glGetUniformLocation( _pbr_shader._program, "uCubeMapFaceNum" ), -1.0 );     
  glUniform1i( glGetUniformLocation( _pbr_shader._program, "_pre_rendu" ), false );     

  glBindVertexArray( _groundVAO );
  glDrawArrays( GL_TRIANGLES, 0, 6 );
  glBindVertexArray( 0 );
  glUseProgram( 0 );
 

  // Draw tables
  // -----------
  _pbr_shader.Use();

  for( int i = 0; i < _tables.size(); i++ )
  {
    Msend = glm::mat4();

    Msend = glm::translate( Msend, _tables[ i ]._position );
    Msend = glm::rotate( Msend, _tables[ i ]._angle, glm::vec3( -1.0, 0.0 , 0.0 ) );
    Msend = glm::scale( Msend, glm::vec3( _tables[ i ]._scale ) * 1.0f ); 

    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, _irradianceMap ); 

    glUniformMatrix4fv( glGetUniformLocation( _pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( viewMatrix ) );
    glUniformMatrix4fv( glGetUniformLocation( _pbr_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( Msend ) );
    glUniformMatrix4fv( glGetUniformLocation( _pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( projectionM ) );

    //glUniformMatrix4fv( glGetUniformLocation(pbr_shader._program, "uLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
    glUniform1i( glGetUniformLocation( _pbr_shader._program, "uLightCount" ), _lights.size() );

    for( int i = 0; i < _lights.size(); i++ )
    {
      string temp = to_string( i );
      glUniform3fv( glGetUniformLocation( _pbr_shader._program, ( "uLightPos["+ temp +"]" ).c_str() ),1, &_lights[ i ]._position[ 0 ] );
      glUniform3fv( glGetUniformLocation( _pbr_shader._program, ( "LightColor["+temp+"]" ).c_str() ),1, &_lights[ i ]._color[ 0 ] );
    }

    glUniform1f( glGetUniformLocation( _pbr_shader._program, "ambientSTR" ), _tables[ i ]._ambient_str );
    glUniform1f( glGetUniformLocation( _pbr_shader._program, "diffuseSTR" ), _tables[ i ]._diffuse_str );
    glUniform1f( glGetUniformLocation( _pbr_shader._program, "specularSTR" ), _tables[ i ]._specular_str );
    glUniform1f( glGetUniformLocation( _pbr_shader._program, "uShiniSTR" ), _tables[ i ]._shini_str );

    glUniform1i( glGetUniformLocation( _pbr_shader._program, "uBloom" ), _tables[ i ]._bloom );
    glUniform1f( glGetUniformLocation( _pbr_shader._program, "uBloomBrightness" ), _tables[ i ]._bloom_brightness );

    glUniform3fv( glGetUniformLocation( _pbr_shader._program, "uViewPos" ), 1, &_camera->_position[ 0 ] );

    glUniform1f( glGetUniformLocation( _pbr_shader._program, "uAlpha" ), _tables[ i ]._alpha );
    glUniform1f( glGetUniformLocation( _pbr_shader._program, "uID" ), _tables[ i ]._id );    
    glUniform1f( glGetUniformLocation( _pbr_shader._program, "uCubeMapFaceNum" ), -1.0 ); 

    _table_model->Draw( _pbr_shader );
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

    // Bind classic texture where we gonna render
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

void Scene::BlurProcess()
{ 
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );   

  bool first_ite = true;
  int horizontal = 1; 
  GLuint amount = 6;
  _blur_shader.Use();
  
  for( GLuint i = 0; i < amount; i++ )
  {
    glBindFramebuffer( GL_FRAMEBUFFER, _window->_toolbox->_pingpongFBO[ horizontal ] ); 
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    horizontal = ( horizontal == 0 ) ? 1 : 0;

    if( !first_ite )
    {
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_pingpongColorbuffers[ horizontal ] );
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

void Scene::BloomProcess()
{
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  _bloom_shader.Use();

  glActiveTexture( GL_TEXTURE0 );
  
  if( _multi_sample )
  {
    glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_final_tex_color_buffer[ 0 ] );
  }
  else
  {
    glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_temp_tex_color_buffer[ 0 ] );
  }
  glActiveTexture( GL_TEXTURE1 );
  glBindTexture( GL_TEXTURE_2D, _window->_toolbox->_pingpongColorbuffers[ 0 ] );
  glUniform1i( glGetUniformLocation( _bloom_shader._program, "uBloom" ), _bloom );
  glUniform1f( glGetUniformLocation( _bloom_shader._program, "uExposure" ), _exposure );
  _window->_toolbox->RenderQuad();
}