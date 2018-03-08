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
#include <vector>

#include <SDL2/SDL.h>

using namespace std;


//******************************************************************************
//**********  Class Camera  ****************************************************
//******************************************************************************
class Scene;


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
            float     iMoveSpeed,
            bool      iDemoScript,
            Scene *   iScene );

    void CameraUpdate( float iDeltaTime );

    void KeyboardPositionUpdate( float iDeltaTime );

    void MouseUpdate();

    void FrontUpdate();

    void PrintState();

    void SetProjectionMatrix( glm::mat4 * iProjectionMatrix );

    void UpdateViewMatrix();

    void DemoScript( float iDeltaTime );

    void InitBezierData();

    float BezierCalculation( float A,
                             float B,
                             float C,
                             float D,
                             float t );
    
 
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

    bool         _demo_script;
    float        _bezier_acc;
    float        _bezier_speed;
    float        _current_speed;
    unsigned int _bezier_step;
    float        _current_time;
    std::vector < std::vector< glm::vec3 > > _demo_bezier_data;

    Scene * _scene;
     
};

#endif  // CAMERA_H