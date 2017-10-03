#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <iostream>

#include <SDL2/SDL.h>

using namespace std;


//******************************************************************************
//**********  Class Camera  ****************************************************
//******************************************************************************

class Camera
{

  public:


    // Camera functions
    // ---------------

    Camera();

    void PrintState();

    void KeyboardPositionUpdate( float iDeltaTime );

    void MouseFrontUpdate();

 
    // Camera class members
    // --------------------
    
    glm::vec3 _position;   
    glm::vec3 _front;
    glm::vec3 _up;    
    
    float _near; 
    float _far; 
    float _yaw; 
    float _pitch;

    float _move_speed; 

    int _Z_state;
    int _D_state;
    int _Q_state;
    int _S_state;
 
};