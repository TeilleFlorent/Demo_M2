#include "main.hpp"


//******************************************************************************
//**********  Global variables  ************************************************
//******************************************************************************

// Shaders
Shader pbr_shader;
Shader skybox_shader;
Shader flat_color_shader;
Shader observer_shader;
Shader blur_shader;
Shader bloom_shader;
Shader blit_shader;
Shader cube_map_converter_shader;
Shader diffuse_irradiance_shader;

// Models
Model table_model;

// VAOs
static GLuint skyboxVAO = 0;
static GLuint lampVAO = 0;
static GLuint observerVAO = 0;
GLuint quadVAO = 0;
static GLuint groundVAO = 0;
static GLuint cubeVAO = 0;

// VBOs
static GLuint skyboxVBO = 0;
static GLuint lampVBO = 0;
static GLuint observerVBO = 0;
GLuint quadVBO;
static GLuint groundVBO = 0;
static GLuint cubeVBO = 0;

// FBOs & RBOs
// -----------
GLuint pingpongFBO[2];

GLuint hdrFBO;
GLuint dephtRBO;

GLuint final_hdr_FBO;
GLuint final_depht_RBO;

unsigned int captureFBO;
unsigned int captureRBO;


// Textures
// --------
static unsigned int hdrTexture;
static unsigned int envCubemap;
static unsigned int irradianceMap;   
std::vector< const GLchar * > faces; // data skybox cube map texture

static GLuint pingpongColorbuffers[2];
static GLuint temp_tex_color_buffer[2];
static GLuint final_tex_color_buffer[2];

static GLuint tex_albedo_ground = 0;
static GLuint tex_normal_ground = 0;
static GLuint tex_height_ground = 0;
static GLuint tex_AO_ground = 0;
static GLuint tex_roughness_ground = 0;
static GLuint tex_metalness_ground = 0;

float depth_map_res_seed = /*2048.0*/ 1024.0;
float depth_map_res_x, depth_map_res_y, depth_map_res_x_house, depth_map_res_y_house;

float reflection_cubeMap_res = /*2048.0*/ 512;
float tex_VL_res_seed = 2048.0;
float tex_VL_res_x, tex_VL_res_y;

// Lights
static Light * lights;
static int nb_lights;

// Window
Window * _window;

// Toolbox
Toolbox * _toolbox;

// Camera
Camera * _camera;

// Clock
Clock * _clock;

// HDRManager
HDRManager * _hdr_image_manager;

// All scene's objects
static int nb_table = 3;
static Object * table;
static Object * ground1;

// Sphere param
static int longi = 20;
static int lati = 20;
static int nbVerticesSphere;

// Bloom param
static float exposure = 0.65;
static bool bloom = false;
static float bloom_downsample = 0.5;

// Multi sample param
static bool multi_sample = false;
static int nb_multi_sample = 4;

// IBL param
static int res_IBL_cubeMap = 512;
static int res_irradiance_cubeMap = 32;


//******************************************************************************
//**********  Pipeline functions  **********************************************
//******************************************************************************

int main()
{
  // Init srand seed
  srand( time( NULL ) );

  // Create and init window
  _window = new Window();

  // Create toolbox
  _toolbox = new Toolbox();

  // Create and init scene camera
  _camera = new Camera();
  
  // Create and init scene clock
  _clock = new Clock();

  // Create HDR Image Manager
  _hdr_image_manager = new HDRManager();


  // Compilation des shaders
  // -----------------------
  pbr_shader.SetShaderClassicPipeline(                "../shaders/pbr_lighting.vs",       "../shaders/pbr_lighting.fs" );
  skybox_shader.SetShaderClassicPipeline(             "../shaders/skybox.vs",             "../shaders/skybox.fs" );
  flat_color_shader.SetShaderClassicPipeline(         "../shaders/flat_color.vs",         "../shaders/flat_color.fs" );
  observer_shader.SetShaderClassicPipeline(           "../shaders/observer.vs",           "../shaders/observer.fs" );
  blur_shader.SetShaderClassicPipeline(               "../shaders/blur.vs",               "../shaders/blur.fs" );
  bloom_shader.SetShaderClassicPipeline(              "../shaders/bloom_blending.vs",     "../shaders/bloom_blending.fs" );
  blit_shader.SetShaderClassicPipeline(               "../shaders/blit.vs",               "../shaders/blit.fs" );
  cube_map_converter_shader.SetShaderClassicPipeline( "../shaders/cube_map_converter.vs", "../shaders/cube_map_converter.fs" );
  diffuse_irradiance_shader.SetShaderClassicPipeline( "../shaders/cube_map_converter.vs", "../shaders/diffuse_irradiance.fs" );


  // Set texture uniform location
  // ----------------------------
  pbr_shader.Use();
  glUniform1i( glGetUniformLocation( pbr_shader._program, "uTextureDiffuse1" ), 0 ) ;
  glUniform1i( glGetUniformLocation( pbr_shader._program, "uTextureNormal1" ), 1 );
  glUniform1i( glGetUniformLocation( pbr_shader._program, "uTextureHeight1" ), 2 ) ;
  glUniform1i( glGetUniformLocation( pbr_shader._program, "uTextureAO1" ), 3 );
  glUniform1i( glGetUniformLocation( pbr_shader._program, "uTextureRoughness1" ), 4 );
  glUniform1i( glGetUniformLocation( pbr_shader._program, "uTextureMetalness1" ), 5 );
  glUniform1i( glGetUniformLocation( pbr_shader._program, "uTextureSpecular1" ), 6 );

  glUniform1i( glGetUniformLocation( pbr_shader._program, "uIrradianceCubeMap" ), 9 ); 
  glUseProgram( 0 );

  skybox_shader.Use();
  glUniform1i( glGetUniformLocation( skybox_shader._program, "uSkyboxTexture" ), 0 );
  glUseProgram( 0 );

  observer_shader.Use();
  glUniform1i( glGetUniformLocation( observer_shader._program, "uTexture1" ), 0 );
  glUseProgram( 0 );

  blit_shader.Use();
  glUniform1i( glGetUniformLocation( blit_shader._program, "uTexture1" ), 0 );
  glUseProgram( 0 );

  blur_shader.Use();
  glUniform1i( glGetUniformLocation( blur_shader._program, "uTexture" ), 0 );
  glUseProgram( 0 );

  bloom_shader.Use();
  glUniform1i( glGetUniformLocation( bloom_shader._program, "uBaseColorTexture" ), 0 );
  glUniform1i( glGetUniformLocation( bloom_shader._program, "uBloomBrightnessTexture" ), 1 );
  glUseProgram( 0 );


  // Scene models loading
  // --------------------
  table_model.Load_Model( "../Models/cube/Rounded Cube.fbx", 0 );
  table_model.Print_info_model();
  /*table_model.Load_Model("../Models/cube2/Crate_Fragile.3DS", 0);
  table_model.Print_info_model();*/


  // Init scene data
  InitData();

  atexit( Quit );

  // Run the program loop
  Loop( _window->_SDL_window );

  return 0;
}

static void Quit()
{
  std::cout << std::endl << "Program Quit" << std::endl;


  // Delete textures
  // ---------------
  if( pingpongColorbuffers[ 0 ] )
    glDeleteTextures( 1, &pingpongColorbuffers[ 0 ] );
  if( pingpongColorbuffers[ 1 ] )
    glDeleteTextures( 1, &pingpongColorbuffers[ 1 ] );
  if( hdrTexture )
    glDeleteTextures( 1, &hdrTexture );
  if( envCubemap )
    glDeleteTextures( 1, &envCubemap );
  if( irradianceMap )
    glDeleteTextures( 1, &irradianceMap );
  if( temp_tex_color_buffer[ 0 ] )
    glDeleteTextures( 1, &temp_tex_color_buffer[ 0 ] );
  if( temp_tex_color_buffer[ 1 ] )
    glDeleteTextures( 1, &temp_tex_color_buffer[ 1 ] );
  if( final_tex_color_buffer[ 0 ] )
    glDeleteTextures( 1, &final_tex_color_buffer[ 0 ] );
  if( final_tex_color_buffer[ 1 ] )
    glDeleteTextures( 1, &final_tex_color_buffer[ 1 ] );
  if( tex_albedo_ground )
    glDeleteTextures( 1, &tex_albedo_ground );
  if( tex_normal_ground )
    glDeleteTextures( 1, &tex_normal_ground );
  if( tex_height_ground )
    glDeleteTextures( 1, &tex_height_ground );
  if( tex_AO_ground )
    glDeleteTextures( 1, &tex_AO_ground );
  if( tex_roughness_ground )
    glDeleteTextures( 1, &tex_roughness_ground );
  if( tex_metalness_ground )
    glDeleteTextures( 1, &tex_metalness_ground );


  // Delete VAOs
  // -----------
  if( skyboxVAO )
    glDeleteVertexArrays( 1, &skyboxVAO );
  if( lampVAO )
    glDeleteVertexArrays( 1, &lampVAO );
  if( observerVAO )
    glDeleteVertexArrays( 1, &observerVAO );
  if( quadVAO )
    glDeleteVertexArrays( 1, &quadVAO );
  if( groundVAO )
    glDeleteVertexArrays( 1, &groundVAO );
  if( cubeVAO )
    glDeleteVertexArrays( 1, &cubeVAO );
  

  // Delete VBOs
  // -----------
  if( lampVBO )
    glDeleteBuffers( 1, &lampVBO );
  if( skyboxVBO )
    glDeleteBuffers( 1, &skyboxVBO );
  if( observerVBO )
    glDeleteBuffers( 1, &observerVBO );
  if( quadVBO )
    glDeleteBuffers( 1, &quadVBO );
  if( groundVBO )
    glDeleteBuffers( 1, &groundVBO );
  if( cubeVBO )
    glDeleteBuffers( 1, &cubeVBO );


  // Delete FBOs
  // -----------
  if( pingpongFBO[ 0 ] )
    glDeleteFramebuffers( 1, &pingpongFBO[ 0 ] );
  if( pingpongFBO[ 1 ] )
    glDeleteFramebuffers( 1, &pingpongFBO[ 1 ] );
  if( hdrFBO )
    glDeleteFramebuffers( 1, &hdrFBO );
  if( final_hdr_FBO )
    glDeleteFramebuffers( 1, &final_hdr_FBO );
  if( hdrFBO )
    glDeleteFramebuffers( 1, &hdrFBO );
  if( captureFBO )
    glDeleteFramebuffers( 1, &captureFBO );


  // Delete RBOs
  // -----------
  if( dephtRBO )
    glDeleteRenderbuffers( 1, &dephtRBO );
  if( final_depht_RBO )
    glDeleteRenderbuffers( 1, &final_depht_RBO );
  if( captureRBO )
    glDeleteRenderbuffers( 1, &captureRBO );


  // Delete window
  // -------------
  if( _window->_openGL_context )
    SDL_GL_DeleteContext( _window->_openGL_context );
  if( _window->_SDL_window )
    SDL_DestroyWindow( _window->_SDL_window );

}

static void InitData()
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
  GLfloat * dataLamp = _toolbox->BuildSphere( longi,
                                              lati );
  nbVerticesSphere = ( 6 * 3 * longi * lati );


  // Create skybox VAO
  // -----------------
  glGenVertexArrays( 1, &skyboxVAO );
  glBindVertexArray( skyboxVAO );
  glGenBuffers( 1, &skyboxVBO );
  glBindBuffer( GL_ARRAY_BUFFER, skyboxVBO );
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
  faces.push_back( "../skybox/s1/front.png" );
  faces.push_back( "../skybox/s1/back.png" );
  faces.push_back( "../skybox/s1/top.png" );
  faces.push_back( "../skybox/s1/bottom.png" );
  faces.push_back( "../skybox/s1/right.png" );
  faces.push_back( "../skybox/s1/left.png" );


  // IBL all pre process
  // -------------------

  // Load hdr skybox texture
  _hdr_image_manager->stbi_set_flip_vertically_on_load( true );
  int width, height, nrComponents;
  //std::string temp_str("../skybox/hdr skybox 2/Ridgecrest_Road_Ref.hdr");
  //std::string temp_str("../skybox/hdr skybox 1/Arches_E_PineTree_3k.hdr");
  std::string temp_str("../skybox/hdr skybox 3/QueenMary_Chimney_Ref.hdr");
  float * data = _hdr_image_manager->stbi_loadf( temp_str.c_str(), &width, &height, &nrComponents, 0 );
  if( data )
  {
    glGenTextures( 1, &hdrTexture );
    glBindTexture( GL_TEXTURE_2D, hdrTexture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data ); // note how we specify the texture's data value to be float
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    _hdr_image_manager->stbi_image_free( data );
  }
  else
  {
    std::cout << "Failed to load HDR image.\n" << std::endl;
  }

  // Gen FBO and RBO to render hdr tex into cube map tex
  glGenFramebuffers( 1, &captureFBO );
  glGenRenderbuffers( 1, &captureRBO );
  glBindFramebuffer( GL_FRAMEBUFFER, captureFBO );
  glBindRenderbuffer( GL_RENDERBUFFER, captureRBO );
  glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, res_IBL_cubeMap, res_IBL_cubeMap );
  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO );

  // Gen cubemap texture
  glGenTextures( 1, &envCubemap );
  glBindTexture( GL_TEXTURE_CUBE_MAP, envCubemap );
  for( unsigned int i = 0; i < 6; ++i )
  {
    glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, res_IBL_cubeMap, res_IBL_cubeMap, 0, GL_RGB, GL_FLOAT, nullptr );
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
  cube_map_converter_shader.Use();
  glUniform1i(glGetUniformLocation( cube_map_converter_shader._program, "uEquirectangularMap" ), 0 );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, hdrTexture );
  glUniformMatrix4fv( glGetUniformLocation( cube_map_converter_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( captureProjection ) );

  glViewport( 0, 0, res_IBL_cubeMap, res_IBL_cubeMap ); // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer( GL_FRAMEBUFFER, captureFBO );
  for( unsigned int i = 0; i < 6; ++i )
  {
    glUniformMatrix4fv( glGetUniformLocation( cube_map_converter_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( captureViews[ i ] ) );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    RenderCube();
  }
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );

  // Gen irradiance cube map texture
  glGenTextures( 1, &irradianceMap );
  glBindTexture( GL_TEXTURE_CUBE_MAP, irradianceMap );
  for( unsigned int i = 0; i < 6; ++i )
  {
    glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, res_irradiance_cubeMap, res_irradiance_cubeMap, 0, GL_RGB, GL_FLOAT, nullptr );
  }
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  glBindFramebuffer( GL_FRAMEBUFFER, captureFBO );
  glBindRenderbuffer( GL_RENDERBUFFER, captureRBO );
  glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, res_irradiance_cubeMap, res_irradiance_cubeMap );

  // Diffuse irradiance cube map calculation  
  diffuse_irradiance_shader.Use();
  glUniform1i( glGetUniformLocation( diffuse_irradiance_shader._program, "uEnvironmentMap" ), 0 );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, envCubemap );
  glUniformMatrix4fv( glGetUniformLocation( diffuse_irradiance_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( captureProjection ) );

  glViewport( 0, 0, res_irradiance_cubeMap, res_irradiance_cubeMap ); // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer( GL_FRAMEBUFFER, captureFBO );
  for( unsigned int i = 0; i < 6; ++i )
  {
    glUniformMatrix4fv( glGetUniformLocation( diffuse_irradiance_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( captureViews[ i ] ) );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    RenderCube();
  }
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );


  // Create observer VAO
  // -------------------
  glGenVertexArrays( 1, &observerVAO );
  glBindVertexArray( observerVAO );
  glEnableVertexAttribArray( 0 );
  glEnableVertexAttribArray( 1 );
  glGenBuffers( 1, &observerVBO );
  glBindBuffer( GL_ARRAY_BUFFER, observerVBO );
  glBufferData( GL_ARRAY_BUFFER, sizeof observer, observer, GL_STATIC_DRAW );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0 ,( const void * )( 0*( sizeof( float ) ) ) );  
  glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0 ,( const void * )( 12*( sizeof( float ) ) ) );
  glBindBuffer( GL_ARRAY_BUFFER, 0);
  glBindVertexArray( 0);


  // Create lamp VAO
  // ---------------
  glGenVertexArrays( 1, &lampVAO );
  glBindVertexArray( lampVAO);
  glEnableVertexAttribArray( 0 );
  glEnableVertexAttribArray( 1  );
  glGenBuffers( 1, &lampVBO );
  glBindBuffer( GL_ARRAY_BUFFER, lampVBO );
  glBufferData( GL_ARRAY_BUFFER,( ( 6 * 6 * longi * lati ) ) * ( sizeof( float ) ), dataLamp, GL_STATIC_DRAW );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * ( sizeof( float ) ), ( const void * )0 );
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * ( sizeof( float ) ), /*3*(sizeof(float))*/( const void * )0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );


  // Create ground VAO
  // -----------------
  if( groundVAO == 0 )
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
    glGenVertexArrays( 1, &groundVAO );
    glGenBuffers( 1, &groundVBO );
    glBindVertexArray( groundVAO );
    glBindBuffer( GL_ARRAY_BUFFER, groundVBO );
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
  glGenFramebuffers( 1, &hdrFBO );
  glGenRenderbuffers( 1, &dephtRBO );
  glBindFramebuffer( GL_FRAMEBUFFER, hdrFBO );
  glGenTextures( 2, temp_tex_color_buffer );

  if( multi_sample )
  {
    for( GLuint i = 0; i < 2; i++ ) 
    {
      glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[ i ] );
      glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, nb_multi_sample , GL_RGB16F, _window->_width, _window->_height, GL_TRUE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );  
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[ i ], 0 );
    }
    glBindRenderbuffer( GL_RENDERBUFFER, dephtRBO );
    glRenderbufferStorageMultisample( GL_RENDERBUFFER, nb_multi_sample, GL_DEPTH24_STENCIL8, _window->_width, _window->_height ); 
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dephtRBO );

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
      glBindTexture( GL_TEXTURE_2D, temp_tex_color_buffer[ i ] );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, _window->_width, _window->_height, 0, GL_RGB, GL_FLOAT, NULL );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );  
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, temp_tex_color_buffer[ i ], 0 );
    }
    glBindRenderbuffer( GL_RENDERBUFFER, dephtRBO );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _window->_width, _window->_height );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dephtRBO );
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
  glGenFramebuffers( 1, &final_hdr_FBO );
  glGenRenderbuffers( 1, &final_depht_RBO );
  glBindFramebuffer( GL_FRAMEBUFFER, final_hdr_FBO );
  glGenTextures(2, final_tex_color_buffer);

  for( GLuint i = 0; i < 2; i++ ) 
  {
    glBindTexture( GL_TEXTURE_2D, final_tex_color_buffer[ i ] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, _window->_width, _window->_height, 0, GL_RGB, GL_FLOAT, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );  
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, final_tex_color_buffer[ i ], 0 );
  }
  glBindRenderbuffer( GL_RENDERBUFFER, final_depht_RBO );
  glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _window->_width, _window->_height );
  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, final_depht_RBO );
  glBindRenderbuffer( GL_RENDERBUFFER, 0 );
  if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
  {
    std::cout << "Framebuffer not complete!" << std::endl;
  }
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    

  // Create pingpong buffer
  // ----------------------
  glGenFramebuffers( 2, pingpongFBO );
  glGenTextures( 2, pingpongColorbuffers );
  for( GLuint i = 0; i < 2; i++ )
  {
    glBindFramebuffer( GL_FRAMEBUFFER, pingpongFBO[ i ] );
    glBindTexture( GL_TEXTURE_2D, pingpongColorbuffers[ i ] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, _window->_width * bloom_downsample, _window->_height * bloom_downsample, 0, GL_RGB, GL_FLOAT, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[ i ], 0 );
    if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
      std::cout << "Framebuffer not complete!" << std::endl;
    }
  }
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glBindTexture( GL_TEXTURE_2D, 0 );
    

  // Load ground albedo texture
  // ---------------------------- 
  glGenTextures( 1, &tex_albedo_ground );
  glBindTexture( GL_TEXTURE_2D, tex_albedo_ground );
  if( ( t = IMG_Load( "../Textures/ground1/albedo.png" ) ) != NULL )
  {
    if( _toolbox->IsTextureRGBA( t ) )
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
  glGenTextures( 1, &tex_normal_ground );
  glBindTexture( GL_TEXTURE_2D, tex_normal_ground );
  if( ( t = IMG_Load( "../Textures/ground1/normal.png" ) ) != NULL )
  {
    if( _toolbox->IsTextureRGBA( t ) )
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
  glGenTextures( 1, &tex_height_ground );
  glBindTexture( GL_TEXTURE_2D, tex_height_ground );
  if( ( t = IMG_Load( "../Textures/ground1/height.png" ) ) != NULL )
  {
    if( _toolbox->IsTextureRGBA( t ) )
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
  glGenTextures( 1, &tex_AO_ground );
  glBindTexture( GL_TEXTURE_2D, tex_AO_ground );
  if( ( t = IMG_Load( "../Textures/ground1/AO.png" ) ) != NULL ) 
  {
    if( _toolbox->IsTextureRGBA( t ) )
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
  glGenTextures( 1, &tex_roughness_ground );
  glBindTexture( GL_TEXTURE_2D, tex_roughness_ground );
  if( ( t = IMG_Load( "../Textures/ground1/roughness.png" ) ) != NULL ) 
  {
    if( _toolbox->IsTextureRGBA( t ) )
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
  glGenTextures( 1, &tex_metalness_ground );
  glBindTexture( GL_TEXTURE_2D, tex_metalness_ground );
  if( ( t = IMG_Load( "../Textures/ground1/metalness.png" ) ) != NULL ) 
  {
   if( _toolbox->IsTextureRGBA( t ) )
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

  
  // Lights initialization
  // ---------------------
  nb_lights = 1;
  lights = new Light[ nb_lights ];

  lights[ 0 ]._light_color          = glm::vec3( 1.0,1.0,1.0 ) * glm::vec3( 6.0 );
  lights[ 0 ]._light_specular_color = glm::vec3( 1.0,1.0,1.0 ) * glm::vec3( 3.0 );
  lights[ 0 ]._light_pos            = glm::vec3( -7, 12, -10 );
  /*
  lights[ 1 ]._light_color          = glm::vec3( 1.0,1.0,1.0 ) * glm::vec3( 6.0 );
  lights[ 1 ]._light_specular_color = glm::vec3( 1.0,1.0,1.0 ) * glm::vec3( 3.0 );
  lights[ 1 ]._light_pos            = glm::vec3( -7, 12, 10 );
  
  lights[ 2 ]._light_color          = glm::vec3( 1.0,1.0,1.0 ) * glm::vec3( 6.0 );
  lights[ 2 ]._light_specular_color = glm::vec3( 1.0,1.0,1.0 ) * glm::vec3( 3.0 );
  lights[ 2 ]._light_pos            = glm::vec3( 7, 12, -10 );
  
  lights[ 3 ]._light_color          = glm::vec3( 1.0,1.0,1.0 ) * glm::vec3( 6.0 );
  lights[ 3 ]._light_specular_color = glm::vec3( 1.0,1.0,1.0 ) * glm::vec3( 3.0 );
  lights[ 3 ]._light_pos            = glm::vec3( -7, 12, -10 );
  */  
   
  // Objects initialization
  // ----------------------
  ground1 = new Object;
  table   = new Object[ nb_table ];

  // Tables init
  for( int i = 0; i < nb_table; i++ )
  {
    table[ i ]._ambient_str = 0.05;
    table[ i ]._diffuse_str = 0.4;
    table[ i ]._specular_str = 0.2;
    table[ i ]._shini_str = 8; // 4 8 16 ... 256 

    table[ i ]._angle = 0.0;
    table[ i ]._acca = 0.105;
    table[ i ]._id = 0.0;
    table[ i ]._scale = 0.0075;

    table[ i ]._alpha = 1.0;

    if( i == 0 )
    {
      table[ i ]._x = - 0.8;
      table[ i ]._y = 0.188;
      table[ i ]._z = 0.0;
    }
    if( i == 1 )
    {
      table[ i ]._x = 0.0;
      table[ i ]._y = 0.188;
      table[ i ]._z = 0.0;
    }
    if( i == 2 )
    {
      table[ i ]._x = 0.8;
      table[ i ]._y = 0.188;
      table[ i ]._z = 0.0;
    }

    table[ i ]._shadow_darkness = 0.75;
    table[ i ]._normal_mapping = true;
  }

  // Ground1 init
  ground1->_ambient_str = 0.3;
  ground1->_diffuse_str = 0.15 * 0.3;
  ground1->_specular_str = 0.1 * 0.3;
  ground1->_shini_str = 128; // 4 8 16 ... 256 
  
  ground1->_angle = _PI_2;
  ground1->_acca = 0.105;
  ground1->_id = 1.0;
  ground1->_scale = 1.0;

  ground1->_alpha = 1.0;

  ground1->_x = 0.0;
  ground1->_y = 0.0;
  ground1->_z = 0.0;

  ground1->_shadow_darkness = 0.75;
  ground1->_normal_mapping = true;

}

static void Loop( SDL_Window * iWindow ) 
{
  SDL_GL_SetSwapInterval( 1 );
  
  for(;;)
  {
    _window->ManageEvents( _camera );

    _clock->TimeUpdate();
    
    _camera->Update( _clock->_delta_time );

    Draw();

    _toolbox->PrintFPS();

    SDL_GL_SwapWindow( iWindow );
  }
}

static void Draw() 
{
  // Set different projection Matrix
  // -------------------------------
  glm::mat4 projectionM, projectionM2, projectionM3;

  projectionM  = glm::perspective( 45.0f, ( float )_window->_width / ( float )_window->_height, _camera->_near, _camera->_far ); // rendu de base
  projectionM2 = glm::perspective( 45.0f, ( float )depth_map_res_x / ( float )depth_map_res_y, _camera->_near, _camera->_far ); // pre rendu dans depth tex
  projectionM3 = glm::perspective( 45.0f, ( float )depth_map_res_x_house / ( float )depth_map_res_y_house, _camera->_near, _camera->_far ); // pre rendu dans depth tex HOUSE
  

  // Scene rendering
  // --------------- 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

  // Draw observer
  glViewport( 0, 0, _window->_width, _window->_height );
  observer_shader.Use();
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, temp_tex_color_buffer[ 1 ] /*final_tex_color_buffer[0]*/ /*pingpongColorbuffers[0]*/ /*tex_depth_ssr*/ );
  //glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[ 1 ] /*final_tex_color_buffer[0]*/ /*pingpongColorbuffers[0]*/ /*tex_depth_ssr*/ );
  glUniform1f( glGetUniformLocation( observer_shader._program, "uCameraNear" ), _camera->_near );
  glUniform1f( glGetUniformLocation( observer_shader._program, "uCameraFar" ), _camera->_far );
  glBindVertexArray( observerVAO );

  //glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
  glBindVertexArray( 0 );
  glUseProgram( 0 );
 
  // Render scene
  glViewport( 0, 0, _window->_width, _window->_height );
  RenderScene( true );

  // Blur calculation on bright texture
  glViewport( 0, 0, _window->_width * bloom_downsample, _window->_height * bloom_downsample );
  BlurProcess();

  // Bloom blending calculation => final render
  glViewport( 0, 0, _window->_width, _window->_height );
  BloomProcess();

  glUseProgram( 0 );
}

void RenderScene( bool iIsFinalFBO )
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
    if( multi_sample )
    {
      glBindFramebuffer( GL_FRAMEBUFFER, hdrFBO /*final_hdr_FBO*/ );
    }
    else{
      glBindFramebuffer( GL_FRAMEBUFFER, /*final_hdr_FBO*/ hdrFBO );
    }
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  }


  // Draw skybox
  // -----------
  glDepthMask( GL_FALSE ); // desactivé juste pour draw la skybox
  skybox_shader.Use();   
  glm::mat4 SkyboxViewMatrix = glm::mat4( glm::mat3( viewMatrix ) );  // Remove any translation component of the view matrix

  Msend = glm::mat4( 1.0f );
  glUniformMatrix4fv( glGetUniformLocation( skybox_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( projectionM ) );
  glUniformMatrix4fv( glGetUniformLocation( skybox_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( SkyboxViewMatrix ) );
  glUniformMatrix4fv( glGetUniformLocation( skybox_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( Msend ) );
  glUniform1f( glGetUniformLocation( skybox_shader._program, "uAlpha" ), 1.0 );

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, /*irradianceMap*/ envCubemap ); // bind les 6 textures du cube map 

  glBindVertexArray( skyboxVAO );
  glEnable( GL_BLEND );
  glDrawArrays( GL_TRIANGLES, 0, 36 );
  glDisable( GL_BLEND );
  glBindVertexArray( 0 );

  glDepthMask( GL_TRUE );  // réactivé pour draw le reste
  glUseProgram( 0 );


  // Draw lamps
  // ----------
  flat_color_shader.Use();
  glBindVertexArray( lampVAO );

  for( int i = 0; i < nb_lights; i++ )
  {
    Msend= glm::mat4();
    Msend = glm::translate( Msend, lights[ i ]._light_pos );
    Msend = glm::scale( Msend, glm::vec3( 1.0f ) ); 
    glm::vec3 lampColor = lights[ i ]._light_color;
    glUniformMatrix4fv( glGetUniformLocation( flat_color_shader._program, "uViewMatrix" ) , 1, GL_FALSE, glm::value_ptr( viewMatrix ) );
    glUniformMatrix4fv( glGetUniformLocation( flat_color_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( Msend ) );
    glUniformMatrix4fv( glGetUniformLocation( flat_color_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( projectionM ) );
    glUniform3f( glGetUniformLocation( flat_color_shader._program, "uColor" ), lampColor.x,lampColor.y,lampColor.z );
    glDrawArrays( GL_TRIANGLES, 0, nbVerticesSphere );
  }
  glBindVertexArray( 0 );
  glUseProgram( 0 );


  // Draw ground1
  // ------------
  pbr_shader.Use();

  Msend = glm::mat4();

  Msend = glm::translate( Msend, glm::vec3( ground1->_x,ground1->_y,ground1->_z ) );
  Msend = glm::rotate( Msend, ground1->_angle, glm::vec3( -1.0, 0.0 , 0.0 ) );
  Msend = glm::scale( Msend, glm::vec3( ground1->_scale * 2.0f,ground1->_scale * 2.0f, ground1->_scale * 1.0f ) ); 

  projectionM2[ 0 ] = glm::vec4( ( float )( _window->_width / 2.0 ), 0.0, 0.0, ( float )( _window->_width / 2.0 ) );
  projectionM2[ 1 ] = glm::vec4( 0.0, ( float )( _window->_height / 2.0 ), 0.0, ( float )( _window->_height / 2.0 ) );
  projectionM2[ 2 ] = glm::vec4( 0.0, 0.0, 1.0, 0.0 );
  projectionM2[ 3 ] = glm::vec4( 0.0, 0.0, 0.0, 1.0 );

  projectionM2 = glm::transpose( projectionM2 );

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, tex_albedo_ground );  
  glActiveTexture( GL_TEXTURE1 );
  glBindTexture( GL_TEXTURE_2D, tex_normal_ground ); 
  glActiveTexture( GL_TEXTURE2 );
  glBindTexture( GL_TEXTURE_2D, tex_height_ground ); 
  glActiveTexture( GL_TEXTURE3 );
  glBindTexture( GL_TEXTURE_2D, tex_AO_ground ); 
  glActiveTexture( GL_TEXTURE4 );
  glBindTexture( GL_TEXTURE_2D, tex_roughness_ground ); 
  glActiveTexture( GL_TEXTURE5 );
  glBindTexture( GL_TEXTURE_2D, tex_metalness_ground ); 

  glActiveTexture( GL_TEXTURE9 );
  glBindTexture( GL_TEXTURE_CUBE_MAP, irradianceMap /*envCubemap*/ ); // bind les 6 textures du cube map 

  glUniformMatrix4fv( glGetUniformLocation( pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( viewMatrix ) );
  glUniformMatrix4fv( glGetUniformLocation( pbr_shader._program, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr( Msend ) );
  glUniformMatrix4fv( glGetUniformLocation( pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( projectionM ) );

  //glUniformMatrix4fv(glGetUniformLocation(pbr_shader.Program, "uLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));

  glUniform1i( glGetUniformLocation( pbr_shader._program, "uLightCount" ), nb_lights );

  for( int i = 0; i < nb_lights; i++ )
  {
    string temp = to_string( i );
    glUniform3fv( glGetUniformLocation( pbr_shader._program, ( "uLightPos["+ temp +"]" ).c_str() ),1, &lights[ i ]._light_pos[ 0 ] );
    glUniform3fv( glGetUniformLocation( pbr_shader._program, ( "uLightColor["+temp+"]" ).c_str() ),1, &lights[ i ]._light_color[ 0 ] );
    glUniform3fv( glGetUniformLocation( pbr_shader._program, ( "uLightSpecularColor["+temp+"]" ).c_str() ),1, &lights[ i ]._light_specular_color[ 0 ] );
  }

  glUniform1f( glGetUniformLocation( pbr_shader._program, "uAmbientSTR" ), ground1->_ambient_str );
  glUniform1f( glGetUniformLocation( pbr_shader._program, "uDiffuseSTR" ), ground1->_diffuse_str );
  glUniform1f( glGetUniformLocation( pbr_shader._program, "uSpecularSTR" ), ground1->_specular_str );
  glUniform1f( glGetUniformLocation( pbr_shader._program, "uShiniSTR" ), ground1->_shini_str );

  glUniform3fv( glGetUniformLocation(pbr_shader._program, "uViewPos" ), 1, &_camera->_position[ 0 ] );

  glUniform1f( glGetUniformLocation( pbr_shader._program, "uAlpha" ), ground1->_alpha );
  glUniform1f( glGetUniformLocation( pbr_shader._program, "uID" ), ground1->_id );    
  glUniform1f( glGetUniformLocation( pbr_shader._program, "uCubeMapFaceNum" ), -1.0 );     
  glUniform1i( glGetUniformLocation( pbr_shader._program, "_pre_rendu" ), false );     

  glBindVertexArray( groundVAO );
  glDrawArrays( GL_TRIANGLES, 0, 6 );
  glBindVertexArray( 0 );
  glUseProgram( 0 );
 

  // Draw tables
  // -----------
  pbr_shader.Use();

  for( int i = 0; i < nb_table; i++ )
  {
    Msend = glm::mat4();

    Msend = glm::translate( Msend, glm::vec3( table[ i ]._x,table[ i ]._y,table[ i ]._z ) );
    Msend = glm::rotate( Msend, table[ i ]._angle, glm::vec3( -1.0, 0.0 , 0.0 ) );
    Msend = glm::scale( Msend, glm::vec3( table[ i ]._scale ) * 1.0f ); 

    glActiveTexture( GL_TEXTURE9 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, irradianceMap ); 

    glUniformMatrix4fv( glGetUniformLocation( pbr_shader._program, "uViewMatrix" ), 1, GL_FALSE, glm::value_ptr( viewMatrix ) );
    glUniformMatrix4fv( glGetUniformLocation( pbr_shader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( Msend ) );
    glUniformMatrix4fv( glGetUniformLocation( pbr_shader._program, "uProjectionMatrix" ), 1, GL_FALSE, glm::value_ptr( projectionM ) );

    //glUniformMatrix4fv( glGetUniformLocation(pbr_shader._program, "uLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
    glUniform1i( glGetUniformLocation( pbr_shader._program, "uLightCount" ), nb_lights );

    for( int i = 0; i < nb_lights; i++ )
    {
      string temp = to_string( i );
      glUniform3fv( glGetUniformLocation( pbr_shader._program, ( "uLightPos["+ temp +"]" ).c_str() ),1, &lights[ i ]._light_pos[ 0 ] );
      glUniform3fv( glGetUniformLocation( pbr_shader._program, ( "LightColor["+temp+"]" ).c_str() ),1, &lights[ i ]._light_color[ 0 ] );
      glUniform3fv( glGetUniformLocation( pbr_shader._program, ( "LightSpecularColor["+temp+"]" ).c_str() ),1, &lights[ i ]._light_specular_color[ 0 ] );
    }

    glUniform1f( glGetUniformLocation( pbr_shader._program, "ambientSTR" ), table[ i ]._ambient_str );
    glUniform1f( glGetUniformLocation( pbr_shader._program, "diffuseSTR" ), table[ i ]._diffuse_str );
    glUniform1f( glGetUniformLocation( pbr_shader._program, "specularSTR" ), table[ i ]._specular_str );
    glUniform1f( glGetUniformLocation( pbr_shader._program, "uShiniSTR" ), table[ i ]._shini_str );

    glUniform3fv( glGetUniformLocation( pbr_shader._program, "uViewPos" ), 1, &_camera->_position[ 0 ] );

    glUniform1f( glGetUniformLocation( pbr_shader._program, "uAlpha" ), table[ i ]._alpha );
    glUniform1f( glGetUniformLocation( pbr_shader._program, "uID" ), table[ i ]._id );    
    glUniform1f( glGetUniformLocation( pbr_shader._program, "uCubeMapFaceNum" ), -1.0 ); 

    table_model.Draw( pbr_shader );
  } 
  glUseProgram( 0 );

  // Unbinding    
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(0);


  // Blit multi sample texture to classic texture
  // --------------------------------------------
  if( multi_sample )
  {
    blit_shader.Use();

    // Bind classic texture where we gonna render
    glBindFramebuffer( GL_FRAMEBUFFER, final_hdr_FBO );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, final_tex_color_buffer[ 0 ], 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Send multi sample texture we need to convert into classic texture
    glActiveTexture( GL_TEXTURE0) ;
    glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[ 0 ] );
    glUniform1i( glGetUniformLocation( blit_shader._program, "uSampleCount" ), nb_multi_sample );    
    RenderQuad();

    // Same convert with brightness texture ( bloom )
    glBindFramebuffer( GL_FRAMEBUFFER, final_hdr_FBO );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, final_tex_color_buffer[ 1 ], 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[ 1 ] );
    glUniform1i( glGetUniformLocation( blit_shader._program, "uSampleCount" ), nb_multi_sample );
    RenderQuad();
  }
 
  // Unbinding    
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glBindVertexArray( 0 );
  glUseProgram( 0 );
}

void BlurProcess()
{
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );   

  bool first_ite = true;
  int horizontal = 1; 
  GLuint amount = 6;
  blur_shader.Use();
  
  for( GLuint i = 0; i < amount; i++ )
  {
    glBindFramebuffer( GL_FRAMEBUFFER, pingpongFBO[ horizontal ] ); 
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    horizontal = ( horizontal == 0 ) ? 1 : 0;

    if( !first_ite )
    {
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, pingpongColorbuffers[ horizontal ] );
    }
    else
    {
      first_ite = false;
      glActiveTexture( GL_TEXTURE0 );
      
      if( multi_sample )
      {
        glBindTexture( GL_TEXTURE_2D, final_tex_color_buffer[ 1 ] );    
      }
      else
      {
        glBindTexture( GL_TEXTURE_2D, temp_tex_color_buffer[ 1 ] ); 
      }
    }

    glUniform1f( glGetUniformLocation( blur_shader._program, "uHorizontal" ), horizontal );
    glUniform1f( glGetUniformLocation( blur_shader._program, "uOffsetFactor" ), 1.2 );
    RenderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);     
  }

  glUseProgram(0);
}

void BloomProcess()
{
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  bloom_shader.Use();

  glActiveTexture( GL_TEXTURE0 );
  
  if( multi_sample )
  {
    glBindTexture( GL_TEXTURE_2D, final_tex_color_buffer[ 0 ] );
  }
  else
  {
    glBindTexture( GL_TEXTURE_2D, temp_tex_color_buffer[ 0 ] );
  }
  glActiveTexture( GL_TEXTURE1 );
  glBindTexture( GL_TEXTURE_2D, pingpongColorbuffers[ 0 ] );
  glUniform1i( glGetUniformLocation( bloom_shader._program, "uBloom" ), bloom );
  glUniform1f( glGetUniformLocation( bloom_shader._program, "uExposure" ), exposure );
  RenderQuad();
}

void RenderQuad()
{
  if( quadVAO == 0 )
  {
    GLfloat quadVertices[] =
    {
      // Positions        // UV
      -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
       1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
       1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    // Setup plane VAO
    glGenVertexArrays( 1, &quadVAO );
    glGenBuffers( 1, &quadVBO );
    glBindVertexArray( quadVAO );
    glBindBuffer( GL_ARRAY_BUFFER, quadVBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), &quadVertices, GL_STATIC_DRAW );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ), ( GLvoid* )0 );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ), ( GLvoid* )( 3 * sizeof( GLfloat ) ) );
  }
  glBindVertexArray( quadVAO );
  glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
  glBindVertexArray( 0 );
}

void RenderCube()
{
  // Create cube VAO
  // ---------------
  if( cubeVAO == 0 )
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

    glGenVertexArrays( 1, &cubeVAO );
    glGenBuffers( 1, &cubeVBO );
    glBindBuffer( GL_ARRAY_BUFFER, cubeVBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW);
    glBindVertexArray( cubeVAO );
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
  glBindVertexArray( cubeVAO );
  glDrawArrays( GL_TRIANGLES, 0, 36 );
  glBindVertexArray( 0 );
}
