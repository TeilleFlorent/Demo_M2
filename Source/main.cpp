#include "main.hpp"

static SDL_Window * _win = NULL;
static SDL_GLContext _oglContext = NULL;

  // SHADERS
Shader basic_shader;
Shader skybox_shader;
Shader lamp_shader;
Shader screen_shader;
Shader blur_shader;
Shader bloom_shader;
Shader blit_shader;


// MODELS
Model house_model;
Model table_model;

// VAOs
static GLuint skyboxVAO = 0;
static GLuint lampVAO = 0;
static GLuint screenVAO = 0;
GLuint quadVAO = 0;
static GLuint groundVAO = 0;

// VBOs
static GLuint skyboxVBO = 0;
static GLuint lampVBO = 0;
static GLuint screenVBO = 0;
GLuint quadVBO;
static GLuint groundVBO = 0;


// FBO
GLuint pingpongFBO[2];

GLuint hdrFBO;
GLuint dephtRBO;

GLuint final_hdr_FBO;
GLuint final_depht_RBO;

GLuint ssrFBO;
GLuint ssrRBO; 



// all no models textures
static GLuint tex_cube_map = 0;
static GLuint pingpongColorbuffers[2];
static GLuint temp_tex_color_buffer[2];
static GLuint final_tex_color_buffer[2];


static GLuint tex_color_ssr;
static GLuint tex_depth_ssr;


static GLuint tex_albedo_ground2 = 0;
static GLuint tex_normal_ground2 = 0;
static GLuint tex_height_ground2 = 0;
static GLuint tex_AO_ground2 = 0;
static GLuint tex_roughness_ground2 = 0;
static GLuint tex_metalness_ground2 = 0;


float depth_map_res_seed = /*2048.0*/ 1024.0;
float depth_map_res_x, depth_map_res_y, depth_map_res_x_house, depth_map_res_y_house;

float reflection_cubeMap_res = /*2048.0*/ 512;
float tex_VL_res_seed = 2048.0;
float tex_VL_res_x, tex_VL_res_y;


//dimension fenetre SDL
static int w = 800 * 1.5;
static int h = 600 * 1.5;
static int final_w = w;
static int final_h = h;

// camera para
static glm::vec3 cameraPos   = glm::vec3(0.45, 0.45 ,-0.14);
static glm::vec3 cameraFront = glm::vec3(0.53, -0.19, 0.82);
static glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
static float camera_near = 0.1f;
static float camera_far = 100.0f;
static float walk_speed = 0.1;


static float yaw = 56.99;
static float pitch = -11.0;

// LIGHTS
static light * lights;
static int nb_lights = 4;

// All objets
static objet house;
static objet * table;
static int nb_table = 3;
static objet * ground1;
static objet ground2;

//sphere para
static int longi = 20;
static int lati = 20;
static int nbVerticesSphere;

// camera 
int cameraZ = 0;
int cameraD = 0;
int cameraQ = 0;
int cameraS = 0;

//FLY
GLboolean fly_state = true;

// data cube map texture
std::vector<const GLchar*> faces;

// BLOOM PARA
static float exposure = 0.65;
static bool bloom = false;
static float bloom_downsample = 0.5;

// PARALLAX PARA
static bool parallax = false;

// MULTI SAMPLE PARA
static bool multi_sample = false;
static int nb_multi_sample = 4;

/////////////////////////////////////////////////////////


// FONCTION MAIN
int main() {

  srand(time(NULL));

 
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "Erreur lors de l'initialisation de SDL :  %s", SDL_GetError());
    return -1;
  }
  atexit(SDL_Quit);


  //initAudio();

  //load_audio();


  if((_win = initWindow(w,h, &_oglContext))) {

   SDL_SetRelativeMouseMode(SDL_TRUE);

    initGL(_win);

    // compilation des shaders
    basic_shader.set_shader("../shaders/basic.vs","../shaders/basic.fs");
    skybox_shader.set_shader("../shaders/skybox.vs","../shaders/skybox.fs");
    lamp_shader.set_shader("../shaders/lamp.vs","../shaders/lamp.fs");
    screen_shader.set_shader("../shaders/screen.vs", "../shaders/screen.fs");
    blur_shader.set_shader("../shaders/blur.vs", "../shaders/blur.fs");
    bloom_shader.set_shader("../shaders/bloom_blending.vs", "../shaders/bloom_blending.fs");
    blit_shader.set_shader("../shaders/blit.vs", "../shaders/blit.fs");


    // Set texture samples
    basic_shader.Use();
    glUniform1i(glGetUniformLocation(basic_shader.Program, "texture_diffuse1"), 0);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "texture_normal1"), 1);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "texture_height1"), 2);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "texture_AO1"), 3);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "texture_roughness1"), 4);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "texture_metalness1"), 5);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "texture_specular1"), 6);


    glUniform1i(glGetUniformLocation(basic_shader.Program, "texture_color_SSR"), 7);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "texture_depth_SSR"), 8); 
    glUseProgram(0);

   
    screen_shader.Use();
    glUniform1i(glGetUniformLocation(screen_shader.Program, "texture1"), 0);
    glUseProgram(0);


    blit_shader.Use();
    glUniform1i(glGetUniformLocation(screen_shader.Program, "texture1"), 0);
    glUseProgram(0);


    blur_shader.Use();
    glUniform1i(glGetUniformLocation(blur_shader.Program, "image"), 0);
    glUseProgram(0);

    bloom_shader.Use();
    glUniform1i(glGetUniformLocation(bloom_shader.Program, "scene_color"), 0);
    glUniform1i(glGetUniformLocation(bloom_shader.Program, "bloom_effect"), 1);
    glUseProgram(0);

    
    
    table_model.Load_Model("../Models/cube/Rounded Cube.fbx", 0);
    table_model.Print_info_model();
    /*table_model.Load_Model("../Models/cube2/Crate_Fragile.3DS", 0);
    table_model.Print_info_model();*/


   

    
    initData();

    loop(_win);

  }
  return 0;
}


static void quit(void) {

  if(pingpongColorbuffers[0])
    glDeleteTextures(1, &pingpongColorbuffers[0]);
  if(pingpongColorbuffers[1])
    glDeleteTextures(1, &pingpongColorbuffers[1]);
  


  if(lampVBO)
    glDeleteBuffers(1, &lampVBO);
  if(skyboxVBO)
    glDeleteBuffers(1, &skyboxVBO);
  


  if(_oglContext)
    SDL_GL_DeleteContext(_oglContext);

  if(_win)
    SDL_DestroyWindow(_win);


}


// fonction qui paramettre la fenetre SDL
static SDL_Window * initWindow(int w, int h, SDL_GLContext * poglContext) {
  SDL_Window * win = NULL;
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);


  if( (win = SDL_CreateWindow("Train", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
    w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | 
        SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/)) == NULL )
    return NULL;
    if( (*poglContext = SDL_GL_CreateContext(win)) == NULL ) {
      SDL_DestroyWindow(win);
      return NULL;
    }


    glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if (err != GLEW_OK)
    exit(1); // or handle the error in a nicer way
    if (!GLEW_VERSION_2_1)  // check that the machine supports the 2.1 API.
    exit(1); // or handle the error in a nicer way


  fprintf(stderr, "Version d'OpenGL : %s\n", glGetString(GL_VERSION));
  fprintf(stderr, "Version de shaders supportes : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));  
  atexit(quit);
  return win;
}




void initAudio() {

  int mixFlags = MIX_INIT_MP3 | MIX_INIT_OGG, res;
  res = Mix_Init(mixFlags);
  if( (res & mixFlags) != mixFlags ) {
    fprintf(stderr, "Mix_Init: Erreur lors de l'initialisation de la bibliothèque SDL_Mixer\n");
    fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
  }

  if(Mix_OpenAudio(44100  /*22050*/ , /*AUDIO_S16LSB*/ MIX_DEFAULT_FORMAT, 2, 1024) < 0){
    printf("BUG init audio\n");  
  //exit(-4);
  }

  Mix_VolumeMusic(MIX_MAX_VOLUME/3);

  Mix_AllocateChannels(10);

}


void load_audio(){



}



// set des paramettre liés à openGL
static void initGL(SDL_Window * win) {


  //glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  //glClearDepth(1.0);

  glDepthFunc(GL_LESS); 
  
  resizeGL(win);

  //glEnable(GL_BLEND);
  //glBlendEquation(GL_FUNC_ADD);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 
  //glEnable(GL_MULTISAMPLE); // active anti aliasing 
  
  //glFrontFace(GL_CCW);
  //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  
}



// fonction qui recupere et paramettre les data
static void initData() {

 float aniso = 0.0f; 

 glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso); // get la valeur pour l'aniso


  SDL_Surface * t = NULL;


 GLfloat skyboxVertices[] = {

      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  // top-right
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,// top-left
            // Front face
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // top-right
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
            // Left face
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
            -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
            // Right face
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-right
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-left
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     
            // Bottom face
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,// bottom-left
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
            -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            // Top face
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right     
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left        

};


GLfloat screen[] ={

  /*-1.0,1.0,0.0,
  -0.4,1.0,0.0,
  -1.0,0.2,0.0,  
  -0.4,0.2,0.0,*/
 
  -1.0,-1.0,0.0,
  1.0,-1.0,0.0,
  -1.0,1.0,0.0,  
  1.0,1.0,0.0,
  
  ///////////////
    
/*  0.0f, 0.0f, 1.0f, 0.0f,      
  0.0f, 1.0f, 1.0f, 1.0f*/

  0.0f, 0.0f, 1.0f, 0.0f, 
  0.0f, 1.0f, 1.0f, 1.0f     
  

};



//SHPERE LAMP
GLfloat * dataLamp = buildSphere(longi, lati);
nbVerticesSphere = (6*3*longi*lati);

//GEN LES VAO
glGenVertexArrays(1, &skyboxVAO);
glGenVertexArrays(1, &lampVAO);
glGenVertexArrays(1, &screenVAO);


// GEN FBO & RBO
glGenFramebuffers(1, &hdrFBO);
glGenRenderbuffers(1, &dephtRBO);
glGenFramebuffers(1, &final_hdr_FBO);
glGenRenderbuffers(1, &final_depht_RBO);

glGenFramebuffers(2, pingpongFBO);
glGenFramebuffers(1, &ssrFBO);
glGenRenderbuffers(1, &ssrRBO);


//////////////////////////////


// skybox VAO
glGenBuffers(1, &skyboxVBO);
glBindVertexArray(skyboxVAO);
glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
 glEnableVertexAttribArray(0);
 glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
 glEnableVertexAttribArray(1);
 glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
 glEnableVertexAttribArray(2);
 glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
glBindVertexArray(0);
glBindBuffer(GL_ARRAY_BUFFER, 0);


 //skybox texture
/*faces.push_back("../skybox/s2/front.png");
faces.push_back("../skybox/s2/back.png");
faces.push_back("../skybox/s2/top.png");
faces.push_back("../skybox/s2/bottom.png");
faces.push_back("../skybox/s2/right.png");
faces.push_back("../skybox/s2/left.png");*/


tex_cube_map = loadCubemap(faces);


// screen VAO
glBindVertexArray(screenVAO);
glEnableVertexAttribArray(0);
glEnableVertexAttribArray(1);
  
glGenBuffers(1, &screenVBO);
glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof screen, screen, GL_STATIC_DRAW);
  
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 ,(const void *)(0*(sizeof(float))));  
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0 ,(const void *)(12*(sizeof(float))));

glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);


// lamp VAO
glBindVertexArray(lampVAO);
glEnableVertexAttribArray(0);
glEnableVertexAttribArray(1);
glGenBuffers(1, &lampVBO);
glBindBuffer(GL_ARRAY_BUFFER, lampVBO);
glBufferData(GL_ARRAY_BUFFER,((6 * 6 * longi * lati)) * (sizeof(float)), dataLamp, GL_STATIC_DRAW);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*(sizeof(float)), (const void *)0);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*(sizeof(float)), /*3*(sizeof(float))*/(const void *)0);
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);

// ground VAO
if (groundVAO == 0)
{
        // positions
  glm::vec3 pos1(-1.0, 1.0, 0.0);
  glm::vec3 pos2(-1.0, -1.0, 0.0);
  glm::vec3 pos3(1.0, -1.0, 0.0);
  glm::vec3 pos4(1.0, 1.0, 0.0);
        // texture coordinates
  glm::vec2 uv1(0.0, 1.0);
  glm::vec2 uv2(0.0, 0.0);
  glm::vec2 uv3(1.0, 0.0);
  glm::vec2 uv4(1.0, 1.0);
        // normal vector
  glm::vec3 nm(0.0, 0.0, 1.0);

        // calculate tangent/bitangent vectors of both triangles
  glm::vec3 tangent1, bitangent1;
  glm::vec3 tangent2, bitangent2;
        // - triangle 1
  glm::vec3 edge1 = pos2 - pos1;
  glm::vec3 edge2 = pos3 - pos1;
  glm::vec2 deltaUV1 = uv2 - uv1;
  glm::vec2 deltaUV2 = uv3 - uv1;

  GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

  tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
  tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
  tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
  tangent1 = glm::normalize(tangent1);

  bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
  bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
  bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
  bitangent1 = glm::normalize(bitangent1);

        // - triangle 2
  edge1 = pos3 - pos1;
  edge2 = pos4 - pos1;
  deltaUV1 = uv3 - uv1;
  deltaUV2 = uv4 - uv1;

  f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

  tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
  tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
  tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
  tangent2 = glm::normalize(tangent2);


  bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
  bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
  bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
  bitangent2 = glm::normalize(bitangent2);

  //std::cout << "TEST = " << bitangent1.x << ", " << bitangent1.y << ", " << bitangent1.z << std::endl;


  GLfloat groundVertices[] = {
            // Positions            // normal         // TexCoords  // Tangent                          // Bitangent
    pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
    pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
    pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

    pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
    pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
    pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
  };
        // Setup plane VAO
  glGenVertexArrays(1, &groundVAO);
  glGenBuffers(1, &groundVBO);
  glBindVertexArray(groundVAO);
  glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), &groundVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(11 * sizeof(GLfloat)));
}


  //////////////////////////////
  
  // TEX SSR PROCESS
  //depth
  glGenTextures(1, &tex_depth_ssr);
  glBindTexture(GL_TEXTURE_2D, tex_depth_ssr);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
  /*glGenTextures(1, &tex_depth_ssr);
  glBindTexture(GL_TEXTURE_2D, tex_depth_ssr);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
  
  // color
  glGenTextures(1, &tex_color_ssr);
  glBindTexture(GL_TEXTURE_2D, tex_color_ssr);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT /*GL_COLOR_ATTACHMENT1*/, GL_TEXTURE_2D, tex_depth_ssr, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color_ssr, 0);

  /*glBindRenderbuffer(GL_RENDERBUFFER, ssrRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ssrRBO);*/

  GLuint attachments[2] = {GL_DEPTH_ATTACHMENT,GL_COLOR_ATTACHMENT0};
  //GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments);

  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "Framebuffer not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0); 


  // TEMP COLOR BUFFERS
  glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
  
  glGenTextures(2, temp_tex_color_buffer);

  if(multi_sample){
    for (GLuint i = 0; i < 2; i++) 
    {
      //glBindTexture(GL_TEXTURE_2D, temp_tex_color_buffer[i]);
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[i]);

      //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
      glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nb_multi_sample , /*GL_RGBA*/ GL_RGB16F, w, h, GL_TRUE);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, temp_tex_color_buffer[i], 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[i], 0);

    }

    glBindRenderbuffer(GL_RENDERBUFFER, dephtRBO);
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, nb_multi_sample, GL_DEPTH24_STENCIL8, w, h); 
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dephtRBO);

    GLuint attachments2[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments2);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }else{
    for (GLuint i = 0; i < 2; i++) 
    {
      glBindTexture(GL_TEXTURE_2D, temp_tex_color_buffer[i]);
      //glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[i]);
    
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
      //glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nb_multi_sample , /*GL_RGBA*/ GL_RGB16F, w, h, GL_TRUE);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, temp_tex_color_buffer[i], 0);
      //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[i], 0);

    }

    glBindRenderbuffer(GL_RENDERBUFFER, dephtRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
    //glRenderbufferStorageMultisample(GL_RENDERBUFFER, nb_multi_sample, GL_DEPTH24_STENCIL8, w, h); 
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dephtRBO);

    GLuint attachments2[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments2);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);    
  }

  // FINAL COLOR BUFFERS
  glBindFramebuffer(GL_FRAMEBUFFER, final_hdr_FBO);
  
  glGenTextures(2, final_tex_color_buffer);

  for (GLuint i = 0; i < 2; i++) 
  {
    glBindTexture(GL_TEXTURE_2D, final_tex_color_buffer[i]);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, final_tex_color_buffer[i], 0);
  
  }

  glBindRenderbuffer(GL_RENDERBUFFER, final_depht_RBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, final_depht_RBO);

  
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "Framebuffer not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  



  ///////////////////////////////

  // TEX BLUR PROCESS
  glGenTextures(2, pingpongColorbuffers);
  for (GLuint i = 0; i < 2; i++)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w * bloom_downsample, h * bloom_downsample, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
  
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer not complete!" << std::endl;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  

  // TEX ALBEDO GROUND2  
  glGenTextures(1, &tex_albedo_ground2);
  glBindTexture(GL_TEXTURE_2D, tex_albedo_ground2);

  if( (t = IMG_Load("../Textures/ground1/albedo.png")) != NULL ) {
    if(is_RGBA(t)){
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
    }else{
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
    }
    SDL_FreeSurface(t);
  } else {
    fprintf(stderr, "Erreur lors du chargement de la texture\n");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  }

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); // anisotropie

  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

   // TEX NORMAL GROUND2 
  glGenTextures(1, &tex_normal_ground2);
  glBindTexture(GL_TEXTURE_2D, tex_normal_ground2);

  if( (t = IMG_Load("../Textures/ground1/normal.png")) != NULL ) {
    if(is_RGBA(t)){
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
    }else{
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
    }
    SDL_FreeSurface(t);
  } else {
    fprintf(stderr, "Erreur lors du chargement de la texture\n");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  }

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); // anisotropie

  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);


   // TEX HEIGHT GROUND2  
  glGenTextures(1, &tex_height_ground2);
  glBindTexture(GL_TEXTURE_2D, tex_height_ground2);

  if( (t = IMG_Load("../Textures/ground1/height.png")) != NULL ) {
    if(is_RGBA(t)){
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
    }else{
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
    }
    SDL_FreeSurface(t);
  } else {
    fprintf(stderr, "Erreur lors du chargement de la texture\n");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  }

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); // anisotropie

  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

   // TEX AO GROUND2  
  glGenTextures(1, &tex_AO_ground2);
  glBindTexture(GL_TEXTURE_2D, tex_AO_ground2);

  if( (t = IMG_Load("../Textures/ground1/AO.png")) != NULL ) {
    if(is_RGBA(t)){
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
    }else{
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
    }
    SDL_FreeSurface(t);
  } else {
    fprintf(stderr, "Erreur lors du chargement de la texture\n");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  }

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); // anisotropie

  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

 // TEX ROUGHNESS GROUND2  
  glGenTextures(1, &tex_roughness_ground2);
  glBindTexture(GL_TEXTURE_2D, tex_roughness_ground2);

  if( (t = IMG_Load("../Textures/ground1/roughness.png")) != NULL ) {
    if(is_RGBA(t)){
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
    }else{
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
    }
    SDL_FreeSurface(t);
  } else {
    fprintf(stderr, "Erreur lors du chargement de la texture\n");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  }

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); // anisotropie

  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

   // TEX METALNESS GROUND2  
  glGenTextures(1, &tex_metalness_ground2);
  glBindTexture(GL_TEXTURE_2D, tex_metalness_ground2);

  if( (t = IMG_Load("../Textures/ground1/metalness.png")) != NULL ) {
   if(is_RGBA(t)){
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
    }else{
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
    }
    SDL_FreeSurface(t);
  } else {
    fprintf(stderr, "Erreur lors du chargement de la texture\n");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  }

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); // anisotropie

  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

//////////////////////////////
// LIGHT INIT
lights = new light[nb_lights];

// sun
lights[0].lightColor = glm::vec3(1.0,1.0,1.0);
lights[0].lightSpecularColor = glm::vec3(1.0,1.0,1.0);
lights[0].lightPos = glm::vec3(7, 12, 10);
lights[0].lightColor*= 3.0;
lights[0].lightSpecularColor*= 3.0;

lights[1].lightColor = glm::vec3(1.0,1.0,1.0);
lights[1].lightSpecularColor = glm::vec3(1.0,1.0,1.0);
lights[1].lightPos = glm::vec3(-7, 12, 10);
lights[1].lightColor*= 3.0;
lights[1].lightSpecularColor*= 3.0;

lights[2].lightColor = glm::vec3(1.0,1.0,1.0);
lights[2].lightSpecularColor = glm::vec3(1.0,1.0,1.0);
lights[2].lightPos = glm::vec3(7, 12, -10);
lights[2].lightColor*= 3.0;
lights[2].lightSpecularColor*= 3.0;

lights[3].lightColor = glm::vec3(1.0,1.0,1.0);
lights[3].lightSpecularColor = glm::vec3(1.0,1.0,1.0);
lights[3].lightPos = glm::vec3(-7, 12, -10);
lights[3].lightColor*= 3.0;
lights[3].lightSpecularColor*= 3.0;
 
 

// OBJECTS INIT
ground1 = new objet[1];
table = new objet[nb_table];


//////////////////////////
 
 ground1->start=0.0;
 ground1->dt=0.0;
 ground1->bouge=0;
 ground1->t=0.0;
 ground1->t0=0.0;


//////////////////////////

 for(int i = 0; i < nb_table; i++){
   table[i].AmbientStr = 0.4;
   table[i].DiffuseStr = 0.4;
   table[i].SpecularStr = 0.2;
   table[i].ShiniStr = 8; // 4 8 16 ... 256 
   table[i].constant = 1.0;
   table[i].linear = 0.014;
   table[i].quadratic = 0.0007;

   table[i].angle=2.47;
   table[i].acca=0.105;
   table[i].var=0.0;
   table[i].scale = 0.0075 /*1.0*/;

   table[i].alpha = 1.0;

   if(i == 0){
     table[i].x = - 0.8;
     table[i].y = 0.188;
     table[i].z = 0.0;
   }
   if(i == 1){
     table[i].x = 0.0;
     table[i].y = 0.188;
     table[i].z = 0.0;
   }
   if(i == 2){
     table[i].x = 0.8;
     table[i].y = 0.188;
     table[i].z = 0.0;
   }
   table[i].start=0.0;
   table[i].dt=0.0;
   table[i].bouge=0;
   table[i].t=0.0;
   table[i].t0=0.0;

   table[i].shadow_darkness = 0.75;
   table[i].parallax_height_scale = 0.017;
   table[i].parallax = false;
 }

 //////////////////////////

 ground2.AmbientStr = 0.3;
 ground2.DiffuseStr = 0.15 * 0.3;
 ground2.SpecularStr = 0.1 * 0.3;
 ground2.ShiniStr = 128; // 4 8 16 ... 256 
 ground2.constant = 1.0;
 ground2.linear = 0.014;
 ground2.quadratic = 0.0007;
 
 ground2.angle= myPI_2;
 ground2.acca=0.105;
 ground2.var=1.0;
 ground2.scale = 1.0;

 ground2.alpha = 1.0;

 ground2.x = 0.0;
 ground2.y = 0.0;
 ground2.z = 0.0;
 
 ground2.start=0.0;
 ground2.dt=0.0;
 ground2.bouge=0;
 ground2.t=0.0;
 ground2.t0=0.0;

 ground2.shadow_darkness = 0.75;
 ground2.parallax_height_scale = 0.02;
 ground2.parallax = false;

}


 static void resizeGL(SDL_Window * win) {

  SDL_GetWindowSize(win, &w, &h);
  //glViewport(0, 0, w, h);

  //std::cout << "W = " << w << ", H = " << h << std::endl;

  SDL_WarpMouseInWindow(win,w/2.0,h/2.0);

}


 static void loop(SDL_Window * win) {


  SDL_GL_SetSwapInterval(1);

  
  for(;;) {


    manageEvents(win);

    mobile_move(ground1,1);

    draw();

    printFPS();

    SDL_GL_SwapWindow(win);


    /*printf("1cameraX = %f, cameraY = %f, cameraZ = %f\n",cameraPos.x,cameraPos.y, cameraPos.z); 
    printf("2cameraX = %f, cameraY = %f, cameraZ = %f\n",cameraFront.x,cameraFront.y, cameraFront.z);
    printf("yaw = %f, pitch = %f\n", yaw, pitch);*/ 

   
    }

  }




 static void manageEvents(SDL_Window * win) {

  SDL_Event event;
  glm::vec3 front;


  GLfloat camera_speed = walk_speed*20;


  if(fly_state == true)
    camera_speed = walk_speed*50;

  //printf("camera_speed = %f\n", camera_speed);


  if(cameraZ == 1){

     cameraPos -= (camera_speed*ground1->dt) * cameraFront;

   }

   if(cameraS == 1){

     cameraPos += (camera_speed*ground1->dt) * cameraFront;

   }

   if(cameraQ == 1){

     cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * (camera_speed*ground1->dt);

   }

   if(cameraD == 1){

     cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * (camera_speed*ground1->dt); 

   }



//////////////////////////////////////



while(SDL_PollEvent(&event))

  switch (event.type) {

    case SDL_KEYDOWN:

    switch(event.key.keysym.sym) {

     case SDLK_ESCAPE:
    
    /* printf("\ntime = %f\n", ground1->t);
     std::cout << "x = " << cameraPos.x << ", y = " <<  cameraPos.y << ", z = " <<  cameraPos.z << std::endl;
     std::cout << "pos = " << trees[0].x << " " << trees[0].y << " " << trees[0].z << std::endl;
     std::cout << "angle = " << shield.angle << std::endl;*/
    
     exit(0);


     case 'z' :
     cameraZ = 1;
     break;

     case 'q' :
     cameraQ = 1;
     break;

     case 's' :
     cameraS = 1;
     break;

     case 'd' :
     cameraD=1;
     break;

     case 'a' :
     /*height_scale += 0.001;
     std::cout << "test = " << height_scale << std::endl;*/

     for(int i = 0 ; i < nb_lights; i++){     
      lights[i].lightPos.y += 0.5;
     }

     /*metalness += 0.01;
     std::cout << "metalness = " << metalness << std::endl;*/
     
     break;

     case 'e' :
     /*height_scale -= 0.001;
     std::cout << "test = " << height_scale << std::endl;*/
     
     /*metalness -= 0.01;
     std::cout << "metalness = " << metalness << std::endl;*/

     for(int i = 0 ; i < nb_lights ; i++){     
      lights[i].lightPos.y -= 0.5;
    }     
     
     /*table[0].y -= 0.01;
     std::cout << "test = " << table[0].y << std::endl;*/
     

     break;

     case 'r' :
     
     
     break;

     case 't' :
     

   
     break;

     case 'y' :
     break;

     case 'w' :
     break;

     case 'x' :
     break;

     
     case 'v' :
      if(bloom){
      bloom = false;
     }else{
      bloom = true;
     }
     break;

     case 'b' :
      if(parallax){
      parallax = false;
     }else{
      parallax = true;
     }
     break;


    default:
    fprintf(stderr, "La touche %s a ete pressee\n",
      SDL_GetKeyName(event.key.keysym.sym));
    break;
  }
  break;

  case SDL_KEYUP:

  switch(event.key.keysym.sym) {

    case 'z' :
    cameraZ = 0;
    break;

    case 'q' :
    cameraQ = 0;
    break;

    case 's' :
    cameraS = 0;
    break;

    case 'd' :
    cameraD = 0;
    break;


  }
  break;



  case SDL_WINDOWEVENT:
  if(event.window.windowID == SDL_GetWindowID(win)) {
    switch (event.window.event)  {
      case SDL_WINDOWEVENT_RESIZED:
      SDL_GetWindowSize(win,&w,&h);
      resizeGL(win);    
      initData();
      
      //SDL_WarpMouseInWindow(_win,w/2.0,h/2.0);

      break;
      case SDL_WINDOWEVENT_CLOSE:
      event.type = SDL_QUIT;
      SDL_PushEvent(&event);
      break;
    }
  }
  break;
  case SDL_QUIT:
  exit(0);

}


/////////////////////////////////////// MOUSE

int x,y;                      

if(true){

SDL_GetRelativeMouseState(&x,&y);

//printf("x = %d, y = %d\n", x, y);


    GLfloat xoffset = (float)x;
    GLfloat yoffset = (float)y*-1; // Reversed since y-coordinates go from bottom to left
    
    GLfloat sensitivity = 0.05; // Change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;


    yaw   += xoffset;
    pitch += yoffset;

     if (pitch > 89.0f)
        pitch = 89.0f;
      if (pitch < -89.0f)
        pitch = -89.0f;

      
      //glm::vec3 front;
      front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
      front.y = sin(glm::radians(pitch));
      front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
      cameraFront = glm::normalize(front);
    }

}


static void draw() {


  glm::mat4 projectionM,projectionM2, projectionM3,Msend,viewMatrix;

  projectionM = glm::perspective(45.0f, /* 4.0f/3.0f */(float)w/(float)h, camera_near, camera_far); // rendu de base
  projectionM2 = glm::perspective(45.0f, (float)depth_map_res_x/(float)depth_map_res_y, camera_near, camera_far); // pre rendu dans depth tex
  projectionM3 = glm::perspective(45.0f, (float)depth_map_res_x_house/(float)depth_map_res_y_house, camera_near, camera_far); // pre rendu dans depth tex HOUSE
  
  
  viewMatrix=glm::lookAt(cameraPos, (cameraPos) + cameraFront, cameraUp); 


 ////////////////////////////
   

 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 


 //DRAW SCREEN
 glViewport(0, 0, w, h);
 screen_shader.Use();
 glActiveTexture(GL_TEXTURE0);
 glBindTexture(GL_TEXTURE_2D, /*temp_tex_color_buffer[0]*/ /*final_tex_color_buffer[0]*/ pingpongColorbuffers[0] /*tex_depth_ssr*/);
 //glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[1] /*final_tex_color_buffer[0]*/ /*pingpongColorbuffers[0]*/ /*tex_depth_ssr*/);


 glUniform1f(glGetUniformLocation(screen_shader.Program, "camera_near"), camera_near);
 glUniform1f(glGetUniformLocation(screen_shader.Program, "camera_far"), camera_far);
 
 glBindVertexArray(screenVAO);

 //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

 glBindVertexArray(0);
 glUseProgram(0);
 
////////////////////////////

//pre rendu pour SSR
 //glViewport(0, 0, w, h);
 //RenderShadowedObjects(false,true);

 // rendu scene initial        
 glViewport(0, 0, w, h);
 RenderShadowedObjects(true,false);

 // blur calculation on bright texture
 glViewport(0, 0, w * bloom_downsample, h * bloom_downsample);
 blur_process();

 // bloom blending calculation => final render
 glViewport(0, 0, w, h);
 bloom_process();
 


 glUseProgram(0);
        

}



void RenderShadowedObjects(bool render_into_finalFBO, bool render_into_ssrFBO){


 glm::mat4 projectionM,Msend,viewMatrix,Msend2, projectionM2, projectionM3;

 projectionM = glm::perspective(45.0f, /* 4.0f/3.0f */(float)w/(float)h, camera_near, camera_far);
 projectionM3 = glm::perspective(45.0f, /* 4.0f/3.0f */(float)w/(float)h, -camera_near, -camera_far);
 viewMatrix=glm::lookAt(cameraPos, (cameraPos) + cameraFront, cameraUp); 


 glm::mat4 lightProjection, lightView, light_space_matrix, skybox_light_space_matrix;
 //GLfloat far =  glm::distance(lights[2].lightPos, glm::vec3(house.x,house.y,house.z)) * 1.2f;
 //lightProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 1.0f, far);
 //lightView = glm::lookAt(lights[2].lightPos, glm::vec3(house.x,house.y,house.z) , glm::vec3(0.0,1.0,0.0));
 //light_space_matrix = lightProjection * lightView;
 glm::vec3 clip_info = computeClipInfo(-camera_near, -camera_far);


  if(render_into_finalFBO){
    if(multi_sample){
       glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO /*final_hdr_FBO*/);
    }else{
       glBindFramebuffer(GL_FRAMEBUFFER, /*final_hdr_FBO*/ hdrFBO);
    }
   
    //glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  if(render_into_ssrFBO){
    glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);

    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  if(!render_into_ssrFBO){
 // DRAW SKYBOX 
 //glDepthMask(GL_FALSE); // desactivé juste pour draw la skybox
 skybox_shader.Use();   
 glm::mat4 SkyboxViewMatrix = glm::mat4(glm::mat3(viewMatrix));  // Remove any translation component of the view matrix

 Msend = glm::mat4(1.0f);
 Msend = glm::translate(Msend, glm::vec3(cameraPos.x, cameraPos.y, cameraPos.z));
 Msend = glm::scale(Msend, glm::vec3(100.0f)); 

 glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(/*SkyboxViewMatrix*/ viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionM));
 glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(Msend));

 glUniform1f(glGetUniformLocation(skybox_shader.Program, "alpha"), 1.0);
 

 glBindVertexArray(skyboxVAO);
 glActiveTexture(GL_TEXTURE0);
 glUniform1i(glGetUniformLocation(skybox_shader.Program, "skybox"), 0); // envoi du sampler cube 
 glBindTexture(GL_TEXTURE_CUBE_MAP, tex_cube_map /*reflection_cubeMap*/ /*tex_shadow_cubeMap*/); // bind les 6 textures du cube map 

 glEnable(GL_BLEND);
//glDrawArrays(GL_TRIANGLES, 0, 36);
 glDisable(GL_BLEND);

 glBindVertexArray(0);
 //glDepthMask(GL_TRUE);  // réactivé pour draw le reste
 glUseProgram(0);


  // DRAW LAMP
 lamp_shader.Use();
 glBindVertexArray(lampVAO);
   
 for(int i = 0; i < nb_lights; i++){

   Msend= glm::mat4();
   Msend = glm::translate(Msend, lights[i].lightPos);
   Msend = glm::scale(Msend, glm::vec3(1.0f)); 

   //std::cout << "TEST lamp = " << i << std::endl;

   glm::vec3 lampColor = lights[i].lightColor;

   glUniformMatrix4fv(glGetUniformLocation(lamp_shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
   glUniformMatrix4fv(glGetUniformLocation(lamp_shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(Msend));
   glUniformMatrix4fv(glGetUniformLocation(lamp_shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionM));
   glUniform3f(glGetUniformLocation(lamp_shader.Program, "lampColor"), lampColor.x,lampColor.z,lampColor.z);

   glDrawArrays(GL_TRIANGLES, 0, nbVerticesSphere);

 }
    glBindVertexArray(0);
    glUseProgram(0);


}


/////////////////////////////////// DRAW HOUSE
 /*basic_shader.Use();

 Msend = glm::mat4();

 Msend = glm::translate(Msend, glm::vec3(house.x,house.y,house.z));
 Msend = glm::rotate(Msend, house.angle, glm::vec3(0.0, 1.0 , 0.0));
 Msend = glm::scale(Msend, glm::vec3(house.scale)); 

 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));
 //glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
 glUniform1f(glGetUniformLocation(basic_shader.Program, "send_bias"), 0.01);



 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[0]"),1, &lights[0].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[0]"),1, &lights[0].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[0]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[0]"),  0.007);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[0]"), 0.0002);


 glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), house.AmbientStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), house.DiffuseStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), house.SpecularStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), house.ShiniStr);

 
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), house.alpha);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), house.var);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 0.0);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0);     



 glEnable(GL_CULL_FACE);
 glCullFace(GL_BACK);
 house_model.Draw(basic_shader, Msend2, false);
 glDisable(GL_CULL_FACE);*/


 //////////// DRAW GROUND 2
 if(!render_into_ssrFBO /*true*/){
 basic_shader.Use();

 Msend = glm::mat4();

 Msend = glm::translate(Msend, glm::vec3(ground2.x,ground2.y,ground2.z));
 Msend = glm::rotate(Msend, ground2.angle, glm::vec3(-1.0, 0.0 , 0.0));
 Msend = glm::scale(Msend, glm::vec3(ground2.scale * 2.0f,ground2.scale * 2.0f, ground2.scale * 1.0f)); 

 projectionM2[0] = glm::vec4((float)(w/2.0), 0.0, 0.0, (float)(w/2.0));
 projectionM2[1] = glm::vec4(0.0, (float)(h/2.0), 0.0, (float)(h/2.0));
 projectionM2[2] = glm::vec4(0.0, 0.0, 1.0, 0.0);
 projectionM2[3] = glm::vec4(0.0, 0.0, 0.0, 1.0);

 
 projectionM2 = glm::transpose(projectionM2);

 //projectionM2 = projectionM2 * projectionM;
 //projectionM2 = projectionM * projectionM2;
 //projectionM2 = projectionM2 * projectionM3;
 //projectionM2 = projectionM3 * projectionM2;


 glActiveTexture(GL_TEXTURE0);
 glBindTexture(GL_TEXTURE_2D, tex_albedo_ground2);  
 glActiveTexture(GL_TEXTURE1);
 glBindTexture(GL_TEXTURE_2D, tex_normal_ground2); 
 glActiveTexture(GL_TEXTURE2);
 glBindTexture(GL_TEXTURE_2D, tex_height_ground2); 
 glActiveTexture(GL_TEXTURE3);
 glBindTexture(GL_TEXTURE_2D, tex_AO_ground2); 
 glActiveTexture(GL_TEXTURE4);
 glBindTexture(GL_TEXTURE_2D, tex_roughness_ground2); 
 glActiveTexture(GL_TEXTURE5);
 glBindTexture(GL_TEXTURE_2D, tex_metalness_ground2); 
 
 glActiveTexture(GL_TEXTURE7);
 glBindTexture(GL_TEXTURE_2D, tex_color_ssr);  
 glActiveTexture(GL_TEXTURE8);
 glBindTexture(GL_TEXTURE_2D, tex_depth_ssr); 

 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix2"), 1, GL_FALSE /*GL_TRUE*/, glm::value_ptr(projectionM2));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix3"), 1, GL_FALSE , glm::value_ptr(projectionM3));
 

 //glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
 glUniform1f(glGetUniformLocation(basic_shader.Program, "camera_near"), camera_near);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "camera_far"), camera_far);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "clip_info"),1, &clip_info[0]);
 

 glUniform1f(glGetUniformLocation(basic_shader.Program, "height_scale"), ground2.parallax_height_scale);
 glUniform1i(glGetUniformLocation(basic_shader.Program, "parallax"), (ground2.parallax & parallax));     

 glUniform1i(glGetUniformLocation(basic_shader.Program, "nb_lights"), nb_lights);

 for(int i = 0; i < nb_lights; i++){
  string temp = to_string(i);

   //std::cout << "test = " << i << std::endl;
   glUniform3fv(glGetUniformLocation(basic_shader.Program, ("LightPos["+ temp +"]").c_str()),1, &lights[i].lightPos[0]);
   glUniform3fv(glGetUniformLocation(basic_shader.Program, ("LightColor["+temp+"]").c_str()),1, &lights[i].lightColor[0]);
   glUniform3fv(glGetUniformLocation(basic_shader.Program, ("LightSpecularColor["+temp+"]").c_str()),1, &lights[i].lightSpecularColor[0]);
 }

 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[0]"), ground2.constant);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[0]"),  ground2.linear);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[0]"), ground2.quadratic);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), ground2.AmbientStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), ground2.DiffuseStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), ground2.SpecularStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), ground2.ShiniStr);


 
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), ground2.alpha);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), ground2.var);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 0.0);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0);     
 glUniform1i(glGetUniformLocation(basic_shader.Program, "SSR_pre_rendu"), render_into_ssrFBO);     
 glUniform1f(glGetUniformLocation(basic_shader.Program, "tex_x_size"), w);     
 glUniform1f(glGetUniformLocation(basic_shader.Program, "tex_y_size"), h);     
 

 glBindVertexArray(groundVAO);
 
 glDrawArrays(GL_TRIANGLES, 0, 6);
 glBindVertexArray(0);
 glUseProgram(0);
 }

//// DRAW TABLE
 basic_shader.Use();

 for(int i = 0; i < nb_table /*1.0*/; i++){
 Msend = glm::mat4();

 Msend = glm::translate(Msend, glm::vec3(table[i].x,table[i].y,table[i].z));
 //Msend = glm::rotate(Msend, table[i].angle, glm::vec3(0.0, 1.0 , 0.0));
 Msend = glm::scale(Msend, glm::vec3(table[i].scale) * 1.0f); 


 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));
 //glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
 glUniform1f(glGetUniformLocation(basic_shader.Program, "camera_near"), camera_near);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "camera_far"), camera_far);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "height_scale"), table[i].parallax_height_scale);
 glUniform1i(glGetUniformLocation(basic_shader.Program, "parallax"), (table[i].parallax & parallax));

 glUniform1i(glGetUniformLocation(basic_shader.Program, "nb_lights"), nb_lights);

 /*glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[0]"),1, &lights[0].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[0]"),1, &lights[0].lightSpecularColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[1]"),1, &lights[1].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[1]"),1, &lights[1].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[1]"),1, &lights[1].lightSpecularColor[0]);*/

 for(int i = 0; i < nb_lights; i++){
  string temp = to_string(i);

  //std::cout << "test = " << temp << std::endl;
   glUniform3fv(glGetUniformLocation(basic_shader.Program, ("LightPos["+ temp +"]").c_str()),1, &lights[i].lightPos[0]);
   glUniform3fv(glGetUniformLocation(basic_shader.Program, ("LightColor["+temp+"]").c_str()),1, &lights[i].lightColor[0]);
   glUniform3fv(glGetUniformLocation(basic_shader.Program, ("LightSpecularColor["+temp+"]").c_str()),1, &lights[i].lightSpecularColor[0]);
 }
 

 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[0]"), table[i].constant);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[0]"),  table[i].linear);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[0]"), table[i].quadratic);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), table[i].AmbientStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), table[i].DiffuseStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), table[i].SpecularStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), table[i].ShiniStr);
 
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), table[i].alpha);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), table[i].var);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 0.0);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0); 
 glUniform1i(glGetUniformLocation(basic_shader.Program, "SSR_pre_rendu"), render_into_ssrFBO);    


 table_model.Draw(basic_shader, Msend2, false);

} 
 glUseProgram(0);

    
 //////////////////////////////////////////////////

 glBindFramebuffer(GL_FRAMEBUFFER, 0);

 glBindVertexArray(0);
 glUseProgram(0);

 // BLIT MULTI SAMPLE COLOR BUFFER INTO CLASSIC COLOR BUFFER
 if(multi_sample){
 
   blit_shader.Use();
 
 // color buffer
   glBindFramebuffer(GL_FRAMEBUFFER, final_hdr_FBO);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, final_tex_color_buffer[0], 0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[0]);
   glUniform1i(glGetUniformLocation(blit_shader.Program, "nb_sample"), nb_multi_sample);    


   RenderQuad();

 // brightness buffer
   glBindFramebuffer(GL_FRAMEBUFFER, final_hdr_FBO);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, final_tex_color_buffer[1], 0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, temp_tex_color_buffer[1]);
   glUniform1i(glGetUniformLocation(blit_shader.Program, "nb_sample"), nb_multi_sample);

   RenderQuad();
 }
 ////////////////////////////////////////////////

 glBindFramebuffer(GL_FRAMEBUFFER, 0);
 glBindVertexArray(0);
 glUseProgram(0);



}


void bloom_process(){

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  bloom_shader.Use();

  glActiveTexture(GL_TEXTURE0);
  if(multi_sample){
    glBindTexture(GL_TEXTURE_2D, final_tex_color_buffer[0]);
  }else{
    glBindTexture(GL_TEXTURE_2D, temp_tex_color_buffer[0]);
  }
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[0]);

  glUniform1i(glGetUniformLocation(bloom_shader.Program, "bloom"), bloom);
  glUniform1f(glGetUniformLocation(bloom_shader.Program, "exposure"), exposure);
  RenderQuad();

}


void blur_process(){

  glBindFramebuffer(GL_FRAMEBUFFER, 0);   

  bool first_ite = true;
  int horizontal = 1; 
  GLuint amount = 6;
  blur_shader.Use();
  
  for (GLuint i = 0; i < amount; i++){

    //std::cout << "TEST = " << i << std::endl;

   glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]); 
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   if(horizontal == 0){
    horizontal = 1;
   }else{ horizontal = 0; }

   if(!first_ite){
     glActiveTexture(GL_TEXTURE0);
     glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[horizontal]);
   }else{
    first_ite = false;
    glActiveTexture(GL_TEXTURE0);
    if(multi_sample){
      glBindTexture(GL_TEXTURE_2D, final_tex_color_buffer[1]);    
    }else{
      glBindTexture(GL_TEXTURE_2D, temp_tex_color_buffer[1]); 
    }
   }

   glUniform1f(glGetUniformLocation(blur_shader.Program, "horizontal"), horizontal);
   glUniform1f(glGetUniformLocation(blur_shader.Program, "offset_factor"), 1.2);
   
   RenderQuad();
   glBindFramebuffer(GL_FRAMEBUFFER, 0);     

 }
 glUseProgram(0);


}

void RenderQuad(){

  if (quadVAO == 0){
  
    GLfloat quadVertices[] = {
      // Positions        // Texture Coords
      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    // Setup plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}



static void printFPS(void) {
  Uint32 t;
  static Uint32 t0 = 0, f = 0;
  f++;
  t = SDL_GetTicks();
  if(t - t0 > 1000) {
      fprintf(stderr, "FPS = %80.2f\n", (1000.0 * f / (t - t0)));
    t0 = t;
    f  = 0;
  }
}



// fonction qui charge la sky box
GLuint loadCubemap(vector<const GLchar*> faces){

  GLuint textureID;
  SDL_Surface * t = NULL;

  glGenTextures(1, &textureID);
  glActiveTexture(GL_TEXTURE0);

  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  for(GLuint i = 0; i < faces.size(); i++)
  {
        //image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
    t = IMG_Load(faces[i]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  return textureID;
}


// fonction qui genere la sphere
static GLfloat * buildSphere(int longitudes, int latitudes) {
 
  int i, j, k;
 
  GLfloat theta, phi, r[2], x[2], y[2], z[2], * data;
  GLfloat c2MPI_Long = 2.0 * myPI / longitudes;
  GLfloat cMPI_Lat = myPI / latitudes;
  //data = malloc(((6 * 6 * longitudes * latitudes )) * sizeof *data);
  data = new float[(6 * 6 * longitudes * latitudes )];
  /* assert(data); */
 
  for(i = 0, k = 0; i < latitudes; i++) {
    phi  = -myPI_2 + i * cMPI_Lat;
    y[0] = sin(phi);
    y[1] = sin(phi + cMPI_Lat);
    r[0] = cos(phi);
    r[1] = cos( phi  + cMPI_Lat);
    for(j = 0; j < longitudes; j++){
      theta = j * c2MPI_Long;
      x[0] = cos(theta);
      x[1] = cos(theta + c2MPI_Long);
      z[0] = sin(theta);
      z[1] = sin(theta + c2MPI_Long);


                  // coordonné de vertex                                                        
      data[k++] = r[0] * x[0]; data[k++] = y[0];  data[k++] = r[0] * z[0];      data[k++] = 0.68;  data[k++] = 0.0;  data[k++] = 0.0;
      data[k++] = r[1] * x[1]; data[k++] = y[1];  data[k++] = r[1] * z[1];      data[k++] = 0.68;  data[k++] = 0.0;  data[k++] = 0.0;
      data[k++] = r[0] * x[1]; data[k++] = y[0];  data[k++] = r[0] * z[1];      data[k++] = 0.68;  data[k++] = 0.0;  data[k++] = 0.0;

      data[k++] = r[0] * x[0]; data[k++] = y[0];  data[k++] = r[0] * z[0];      data[k++] = 0.68;  data[k++] = 0.0;  data[k++] = 0.0;
      data[k++] = r[1] * x[0]; data[k++] = y[1];  data[k++] = r[1] * z[0];      data[k++] = 0.68;  data[k++] = 0.0;  data[k++] = 0.0;
      data[k++] = r[1] * x[1]; data[k++] = y[1];  data[k++] = r[1] * z[1];      data[k++] = 0.68;  data[k++] = 0.0;  data[k++] = 0.0;


    }
  }
  return data;
}




// fonction qui gere colision fenetre + colision entre objet, + gere la gravité des objet + gere tout les deplacement x/y
static void mobile_move(objet * tabl,int nb) {

  static int ft = 0;
  static int start;

  int t, i, j;


  if(ft==0){
    ft=1;
    start = SDL_GetTicks();
  }

  t = (SDL_GetTicks()) - start;                          

  //std::cout << "prog current t = " << t << std::endl;
    

  // boucle gère les variables temporelles de chaque objet
  for(i=0;i<nb;i++){

    j=t-tabl[i].bouge;

    if( j>0 ){

      if(tabl[i].start==0.0){
        tabl[i].start=SDL_GetTicks();
      }

      tabl[i].t =  (double)((SDL_GetTicks()) - tabl[i].start);

      tabl[i].dt= (tabl[i].t0 - tabl[i].t) / 1000.0;

      tabl[i].t0=tabl[i].t;

    }
    
  }


}




