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

    unsigned int LoadCubeMap( std::vector< const char * > iPaths );

    float * BuildSphere( int iLongitudes,
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

    unsigned int CreateTextureFromData( SDL_Surface * iImage,
                                        int           iInternalFormat,
                                        int           iFormat,
                                        bool          iMipmap,
                                        bool          iAnisotropy,
                                        float         iAnisotropyValue );

    unsigned int CreateEmptyTexture( int   iWidth,
                                     int   iHeight,
                                     int   iInternalFormat,
                                     int   iFormat,
                                     bool  iMipmap,
                                     bool  iAnisotropy,
                                     float iAnisotropyValue );

    unsigned int CreateCubeMapTexture( int  iResolution,
                                       bool iMipmap );

    void SetFboTexture( unsigned int iTextureID,
                        int          iFormat,
                        int          iWidth,
                        int          iHeight,
                        GLenum       iAttachment );

    void SetFboMultiSampleTexture( unsigned int iTextureID,
                                   int          iSampleCount,
                                   int          iFormat,
                                   int          iWidth,
                                   int          iHeight,
                                   GLenum       iAttachment );

    void LinkRbo( unsigned int iRboID,
                  int          iWidth,
                  int          iHeight );

    void LinkMultiSampleRbo( unsigned int iRboID,
                             int          iSampleCount, 
                             int          iWidth,
                             int          iHeight );

    glm::mat4 AssimpMatrixToGlmMatrix( const aiMatrix4x4 * iAssimpMatrix );

    void CreatePlaneVAO( unsigned int *                iVAO,
                         unsigned int *                iVBO,
                         unsigned int *                iIBO,
                         std::vector< unsigned int > * iIndices,
                         unsigned int                  iSideVerticeCount,
                         float                         iUvScale );


    // Toolbox class members
    // ---------------------

    // Pointer on the toolbox window
    Window * _window;

    HDRManager * _hdr_image_manager;

    // VAO a VBO
    unsigned int _quad_VAO;
    unsigned int _quad_VBO;

    unsigned int _cube_VAO;
    unsigned int _cube_VBO;

    // FBOs & RBOs
    unsigned int _temp_hdr_FBO;
    unsigned int _temp_depth_RBO;

    unsigned int _final_hdr_FBO;

    unsigned int _pingpong_FBO;

    // FBO's textures
    unsigned int _pingpong_color_buffers[ 2 ];
    unsigned int _temp_tex_color_buffer[ 2 ];
    unsigned int _final_tex_color_buffer[ 2 ];

};

#endif  // TOOLBOX_H
