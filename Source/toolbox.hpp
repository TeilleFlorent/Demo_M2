#ifndef TOOLBOX_H
#define TOOLBOX_H

#include "scene.hpp"
#include "hdr_image_manager.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <GL/glew.h>

#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

#define _PI 3.141593
#define _PI_2 1.570796


//******************************************************************************
//**********  Class Toolbox  ***************************************************
//******************************************************************************

class Window;

class Toolbox
{

  public:


    // Toolbox functions
    // ---------------

    Toolbox( Window * iParentWindow );

    void Quit();

    void PrintFPS();

    GLuint LoadCubeMap( std::vector< const GLchar * > iPaths );

    GLfloat * BuildSphere( int iLongitudes,
                           int iLatitudes );

    float RandFloatRange( float iMin,
                          float iMax );

    void PrintMatrix( glm::mat4 * iMatrix );

    bool IsTextureRGBA( SDL_Surface * t );

    void InitAudio();

    void LoadAudio();

    void RenderQuad();

    void RenderCube();

    void RenderObserver();


    // Toolbox class members
    // ---------------------

    // Pointer on the toolbox window
    Window * _window;

    // Sphere param
    int _sphere_longitude_count;
    int _sphere_latitude_count;
    int _sphere_vertices_count;

    HDRManager * _hdr_image_manager;

    // VAO a VBO
    GLuint _quadVAO;
    GLuint _quadVBO;

    GLuint _cubeVAO;
    GLuint _cubeVBO;

    GLuint _observerVAO;
    GLuint _observerVBO;

    // FBOs & RBOs
    GLuint _temp_hdr_FBO;
    GLuint _temp_depht_RBO;

    GLuint _final_hdr_FBO;
    GLuint _final_depht_RBO;

    unsigned int _captureFBO;
    unsigned int _captureRBO;
    GLuint _pingpong_FBO[ 2 ];

    // Textures
    GLuint _pingpong_color_buffers[ 2 ];
    GLuint _temp_tex_color_buffer[ 2 ];
    GLuint _final_tex_color_buffer[ 2 ];

    // Textures resolution
    float _depth_map_res_seed;
    float _depth_map_res_x, _depth_map_res_y;

    float _reflection_cubeMap_res;
    float _tex_VL_res_seed;

};

#endif  // TOOLBOX_H
