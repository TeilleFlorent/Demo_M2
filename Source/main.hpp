#include "classic_model.hpp"
#include "clock.hpp"
#include "hdr_image_manager.hpp"
#include "camera.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"
#include "glm/ext.hpp"

using namespace std;

#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define myPI 3.141593
#define myPI_2 1.570796


//******************************************************************************
//**********  Class Object  ****************************************************
//******************************************************************************

class Object
{

  public:

    float _angle;
    float _acca;
    float _id;
    float _scale;
    float _x, _y, _z;
    double _alpha;
    float _ambient_str;
    float _diffuse_str;
    float _specular_str;
    int _shini_str;
    float _shadow_darkness;
    bool _normal_mapping;

};


//******************************************************************************
//**********  Class Light  *****************************************************
//******************************************************************************

class Light
{

  public:

    glm::vec3 _light_pos;  
    glm::vec3 _save_light_pos;
    glm::vec3 _light_color;
    glm::vec3 _light_specular_color;

};


//******************************************************************************
//**********  Pipeline Functions  **********************************************
//******************************************************************************

static SDL_Window * InitWindow( int iWidth,
                                int iHeight,
                                SDL_GLContext * iOpenGLContext );

static void Quit();

static void InitGL( SDL_Window * iWindow );

static void InitData();

static void ResizeGL( SDL_Window * iWindow );

static void Loop( SDL_Window * iWindow );

static void ManageEvents( SDL_Window * iWindow );

static void Draw();

static void PrintFPS();

GLuint LoadCubeMap();

static GLfloat * BuildSphere( int iLongitudes,
                              int iLatitudes );

void RenderScene( bool iIsFinalFBO );

void RenderQuad();

void RenderCube();

void BlurProcess();

void BloomProcess();

void InitAudio();

void LoadAudio();

float RandFloatRange( float iMin,
                      float iMax );

void PrintMatrix( glm::mat4 * iMatrix );

bool IsTextureRGBA( SDL_Surface * t );

