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

#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cmath>


// SHADER CLASS
class Shader
{
public:

    GLuint Program;
    
    Shader(){

    }

    void Use();


    void set_shader(const GLchar* vertexPath, const GLchar* fragmentPath);
    
    void set_shader2(const GLchar* vertexPath,const GLchar* geoPath, const GLchar* fragmentPath);
    

};
