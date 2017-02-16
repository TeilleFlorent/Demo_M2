#include "classic_model.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"
#include "glm/ext.hpp"

#include <algorithm>
using namespace std;

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cmath>


#define myPI 3.141593
#define myPI_2 1.570796



struct objet{
  float angle;
  float acca;
  float var;
  float scale;
  float x,y,z;
  float start;
  float dt;
  float bouge;
  float t;
  float t0;
  double alpha;
  float AmbientStr;
  float DiffuseStr;
  float SpecularStr;
  int ShiniStr;
  float constant;
  float linear;
  float quadratic;
  float shadow_darkness;
  float parallax_height_scale;
};
typedef struct objet objet;


struct light{
  glm::vec3 lightPos;  
  glm::vec3 save_lightPos;
  glm::vec3 lightColor;
  glm::vec3 lightSpecularColor;
};
typedef struct light light;


//////////////////////////////////

static SDL_Window * initWindow(int w, int h, SDL_GLContext * poglContext);
static void quit(void);
static void initGL(SDL_Window * win);
static void initData(void);
static void resizeGL(SDL_Window * win);
static void loop(SDL_Window * win);
static void manageEvents(SDL_Window * win);
static void draw(void);
static void printFPS(void);
static void mobile_move(objet*,int);
GLuint loadCubemap(vector<const GLchar*>);
static GLfloat * buildSphere(int, int);
void Pre_rendu_feu(glm::mat4, glm::mat4,float);
void RenderShadowedObjects(bool, bool);
void SetBoneTransform(uint , const glm::mat4&, int);
void camera_script();
void audio_script(int, double);
void fire_script();
void Pre_rendu_cubeMap();
void Pre_rendu_shadow_house(glm::mat4, glm::mat4);
void Pre_rendu_shadow_cubeMap();
void RenderQuad();
void blur_process();
void bloom_process();
void initAudio();
void load_audio();
double bezier(double,double,double,double,double); 

  
/////////////////////////////////
  

float rand_FloatRange(float a, float b)
{
    return ((b-a)*((float)rand()/RAND_MAX))+a;
}


void print_mat4(glm::mat4 matrix){

    const float *pSource = (const float*)glm::value_ptr(matrix);
    
    printf("\n");

    for (int i = 0; i < 4;i++){
        for (int j = 0; j < 4; j++){
            printf("%.3f ", pSource[(4 * j) + i]);
        }
        printf("\n");
    }

    printf("\n");
}


glm::vec3 computeClipInfo(float zn, float zf) { 

  return glm::vec3(zn  * zf, zn - zf, zf);
  //return glm::vec3(zn, -1.0f, +1.0f);
}


