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

class Toolbox
{

  public:


    // Toolbox functions
    // ---------------

    Toolbox();

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


    // Toolbox class members
    // ---------------------

    
};