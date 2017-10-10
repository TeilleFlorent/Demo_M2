#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

using namespace std;


//******************************************************************************
//**********  Class Light  *****************************************************
//******************************************************************************

class Light
{

  public:


    // Light functions
    // ---------------

    Light( glm::vec3 iPosition,
           glm::vec3 iColor,
           float iIntensity );


    // Light class members
    // -------------------
    
    glm::vec3 _position;  
    glm::vec3 _color;
    float     _intensity;

};

#endif  // LIGHT_H
