#ifndef CAMERA_H
#define CAMERA_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"
#include "glm/ext.hpp"

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

    Camera( glm::vec3 iPosition,
            glm::vec3 iFront,
            glm::vec3 iUp,
            float     iYaw,
            float     iPitch,
            float     iNear,
            float     iFar,
            float     iFov,
            float     iWidth,
            float     iHeight,
            float     iMoveSpeed );

    void CameraUpdate( float iDeltaTime );

    void KeyboardPositionUpdate( float iDeltaTime );

    void MouseFrontUpdate();

    void PrintState();

    void SetProjectionMatrix( glm::mat4 * iProjectionMatrix );

    void UpdateViewMatrix();
    
 
    // Camera class members
    // --------------------
    
    glm::vec3 _position;   
    glm::vec3 _front;
    glm::vec3 _up;    
    
    float _near; 
    float _far; 
    float _yaw; 
    float _pitch;

    glm::mat4 _projection_matrix;
    glm::mat4 _view_matrix;

    float _move_speed; 

    int _Z_state;
    int _D_state;
    int _Q_state;
    int _S_state;
 
};

#endif  // CAMERA_H