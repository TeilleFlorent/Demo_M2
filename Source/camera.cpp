#include "camera.hpp"
#include "scene.hpp"
#include "window.hpp"
#include <SDL2/SDL_mixer.h>


//******************************************************************************
//**********  Class Camera  ****************************************************
//******************************************************************************

Camera::Camera( glm::vec3 iPosition,
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
                Scene *   iScene )
{
  _scene = iScene;

  _position = iPosition;
  _front    = iFront;
  _up       = iUp;     
    
  _near       = iNear;
  _far        = iFar;
  _yaw        = iYaw;
  _pitch      = iPitch;

  _projection_matrix = glm::perspective( iFov, iWidth / iHeight, iNear, iFar );

  _move_speed = iMoveSpeed;

  _Z_state = 0;
  _D_state = 0;
  _Q_state = 0;
  _S_state = 0;

  _demo_script = iDemoScript;

  if( _demo_script )
  {
    InitBezierData();
  }

  UpdateViewMatrix();
}

void Camera::CameraUpdate( float iDeltaTime )
{
  if( _demo_script )
  {
    DemoScript( iDeltaTime );
  }
  else
  {
    KeyboardPositionUpdate( iDeltaTime );
    MouseUpdate();  
  }
  
  UpdateViewMatrix();
}

void Camera::InitBezierData()
{
  _bezier_acc    = 0.0;
  _bezier_step   = 0;
  _bezier_speed  = 1.0; 
  _current_speed = 0.0;
  _current_time  = 0.0;

  std::vector < glm::vec3 > positions;
  positions.resize( 8 );

  // 0
  positions[ 0 ] = glm::vec3( -10.7526, 1.5, -0.0499162 );
  positions[ 1 ] = glm::vec3( -10.7526, 1.0, -0.0536648 );
  positions[ 2 ] = glm::vec3( -10.7526, 0.5, -0.0517071 );
  positions[ 3 ] = glm::vec3( -10.7526, 0.44, -0.042658 );
  positions[ 4 ] = glm::vec3( 358, -6, 0.0 );
  positions[ 5 ] = glm::vec3( 310, -5, 0.0 );
  positions[ 6 ] = glm::vec3( 395, 0.75, 0.0 );
  positions[ 7 ] = glm::vec3( 360.3, -0.36, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 1
  positions[ 0 ] = glm::vec3( -10.7526, 0.445425, -0.0499162 );
  positions[ 1 ] = glm::vec3( -9.24771, 1.1407, -0.0536648 );
  positions[ 2 ] = glm::vec3( -8.30806, 1.16629, -0.0517071 );
  positions[ 3 ] = glm::vec3( -7.6752, 1.28052, -0.042658 );
  positions[ 4 ] = glm::vec3( 360.3, -0.36, 0.0 );
  positions[ 5 ] = glm::vec3( 360.7, 3.71, 0.0 );
  positions[ 6 ] = glm::vec3( 361.5, -0.48, 0.0 );
  positions[ 7 ] = glm::vec3( 362.6, -3.9, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 2
  positions[ 0 ] = glm::vec3( -7.6752, 1.28052, -0.042658 );
  positions[ 1 ] = glm::vec3( -5.90024, 0.374963, 0.212567 );
  positions[ 2 ] = glm::vec3( -4.86183, 0.3656, 1.04364 );
  positions[ 3 ] = glm::vec3( -3.66525, 0.625211, -0.397205 );
  positions[ 4 ] = glm::vec3( 362.6, -3.9, 0.0 );
  positions[ 5 ] = glm::vec3( 361, -30, 0.0 );
  positions[ 6 ] = glm::vec3( 326, -16, 0.0 );
  positions[ 7 ] = glm::vec3( 404, -2, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 3
  positions[ 0 ] = glm::vec3( -3.66525, 0.625211, -0.397205 );
  positions[ 1 ] = glm::vec3( -3.33225, 1.18308, 3.90641 );
  positions[ 2 ] = glm::vec3( -1.26554, 1.14788, 2.98665 );
  positions[ 3 ] = glm::vec3( -0.0690293, 1.1899, 2.96282 );
  positions[ 4 ] = glm::vec3( 404, -2, 0.0 );
  positions[ 5 ] = glm::vec3( 345, -47, 0.0 );
  positions[ 6 ] = glm::vec3( 269, -18, 0.0 );
  positions[ 7 ] = glm::vec3( 255, -26, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 4
  positions[ 0 ] = glm::vec3( -0.0690293, 1.1899, 2.96282 );
  positions[ 1 ] = glm::vec3( -0.860541, 1.02098, 1.77792 );
  positions[ 2 ] = glm::vec3( -2.10028, 0.953931, 0.523306 );
  positions[ 3 ] = glm::vec3( -1.65892, 0.82473, -0.897429 );
  positions[ 4 ] = glm::vec3( 255, -26, 0.0 );
  positions[ 5 ] = glm::vec3( 270, -9, 0.0 );
  positions[ 6 ] = glm::vec3( 316, -4, 0.0 );
  positions[ 7 ] = glm::vec3( 355, -10, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 5
  positions[ 0 ] = glm::vec3( -1.65892, 0.82473, -0.897429 );
  positions[ 1 ] = glm::vec3( -0.948801, 0.808958, -3.25162 );
  positions[ 2 ] = glm::vec3( 1.44854, 0.791079, 0.258758 );
  positions[ 3 ] = glm::vec3( -1.14998, 0.746634, -0.616307 );
  positions[ 4 ] = glm::vec3( 355, -10, 0.0 );
  positions[ 5 ] = glm::vec3( 434, -11, 0.0 );
  positions[ 6 ] = glm::vec3( 557, -24, 0.0 );
  positions[ 7 ] = glm::vec3( 700, -11, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 6
  positions[ 0 ] = glm::vec3( -1.14998, 0.746634, -0.616307 );
  positions[ 1 ] = glm::vec3( 0.133885, 0.637213, -1.12437 );
  positions[ 2 ] = glm::vec3( 0.880257, 0.935606, -1.89059 );
  positions[ 3 ] = glm::vec3( 1.18115, 1.42267, -2.13825 );
  positions[ 4 ] = glm::vec3( 700, -11, 0.0 );
  positions[ 5 ] = glm::vec3( 696, 1, 0.0 );
  positions[ 6 ] = glm::vec3( 712, 30, 0.0 );
  positions[ 7 ] = glm::vec3( 748, 6, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 7
  positions[ 0 ] = glm::vec3( 1.18115, 1.42267, -2.13825 );
  positions[ 1 ] = glm::vec3( 0.83311, 0.895661, -0.887074 );
  positions[ 2 ] = glm::vec3( 1.17283, 0.596006, 0.271447 );
  positions[ 3 ] = glm::vec3( 1.86791, 0.548586, 1.21698 );
  positions[ 4 ] = glm::vec3( 748, 6, 0.0 );
  positions[ 5 ] = glm::vec3( 728, -3, 0.0 );
  positions[ 6 ] = glm::vec3( 710, -7, 0.0 );
  positions[ 7 ] = glm::vec3( 664, -11, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 8
  positions[ 0 ] = glm::vec3( 1.86791, 0.548586, 1.21698 );
  positions[ 1 ] = glm::vec3( 0.3, 1.40513, 3.77224 );
  positions[ 2 ] = glm::vec3( 3.20213, 1.28838, 3.14024 );
  positions[ 3 ] = glm::vec3( 2.9911, 1.15809, 2.19225 );
  positions[ 4 ] = glm::vec3( 664, -11, 0.0 );
  positions[ 5 ] = glm::vec3( 682, -12, 0.0 );
  positions[ 6 ] = glm::vec3( 599, -10, 0.0 );
  positions[ 7 ] = glm::vec3( 582, -20, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 9
  positions[ 0 ] = glm::vec3( 2.9911, 1.15809, 2.19225 );
  positions[ 1 ] = glm::vec3( 1.89646, 1.2, 1.0131 );
  positions[ 2 ] = glm::vec3( 1.01753, -0.8, -0.170797 );
  positions[ 3 ] = glm::vec3( 0.117208, 1.26884, -2.54577 );
  positions[ 4 ] = glm::vec3( 582, -20, 0.0 );
  positions[ 5 ] = glm::vec3( 598, -10, 0.0 );
  positions[ 6 ] = glm::vec3( 607, 0.0, 0.0 );
  positions[ 7 ] = glm::vec3( 628, -3, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 10
  positions[ 0 ] = glm::vec3( 0.117208, 1.26884, -2.54577 );
  positions[ 1 ] = glm::vec3( 0.048442, 1.26095, -4.84248 );
  positions[ 2 ] = glm::vec3( 0.148109, 1.24751, -7.88722 );
  positions[ 3 ] = glm::vec3( -0.0715887, 1.24279, -11.0367 );
  positions[ 4 ] = glm::vec3( 628, -3, 0.0 );
  positions[ 5 ] = glm::vec3( 630, -5, 0.0 );
  positions[ 6 ] = glm::vec3( 630, 0.0, 0.0 );
  positions[ 7 ] = glm::vec3( 631, -2, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 11
  positions[ 0 ] = glm::vec3( -0.0715887, 1.24279, -11.0367 );
  positions[ 1 ] = glm::vec3( 0.627277, 0.945186, -13.121 );
  positions[ 2 ] = glm::vec3( 1.26551, 0.965567, -14.1112 );
  positions[ 3 ] = glm::vec3( 0.0431405, 1.01521, -15.2657 );
  positions[ 4 ] = glm::vec3( 631, -2, 0.0 );
  positions[ 5 ] = glm::vec3( 598, -7, 0.0 );
  positions[ 6 ] = glm::vec3( 567, -14, 0.0 );
  positions[ 7 ] = glm::vec3( 584, 10, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 12
  positions[ 0 ] = glm::vec3( 0.0431405, 1.01521, -15.2657 );
  positions[ 1 ] = glm::vec3( -0.834547, 1.41769, -19.0219 );
  positions[ 2 ] = glm::vec3( -2.61529, 1.39748, -18.3469 );
  positions[ 3 ] = glm::vec3( -2.37522, 1.56661, -16.2699 );
  positions[ 4 ] = glm::vec3( 584, 10, 0.0 );
  positions[ 5 ] = glm::vec3( 497, -10, 0.0 );
  positions[ 6 ] = glm::vec3( 457, -40, 0.0 );
  positions[ 7 ] = glm::vec3( 325, -30, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 13
  positions[ 0 ] = glm::vec3( -2.37522, 1.56661, -16.2699 );
  positions[ 1 ] = glm::vec3( -0.69252, 1.42972, -16.437 );
  positions[ 2 ] = glm::vec3( 0.496881, 1.25566, -17.5224 );
  positions[ 3 ] = glm::vec3( 0.545994, 0.83996, -18.9886 );
  positions[ 4 ] = glm::vec3( 325, -30, 0.0 );
  positions[ 5 ] = glm::vec3( 304, -11, 0.0 );
  positions[ 6 ] = glm::vec3( 288, -5, 0.0 );
  positions[ 7 ] = glm::vec3( 265, -16, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 14
  positions[ 0 ] = glm::vec3( 0.545994, 0.83996, -18.9886 );
  positions[ 1 ] = glm::vec3( 3.33153, 0.747928, -20.2175 );
  positions[ 2 ] = glm::vec3( 2.99539, 0.777976, -22.9405 );
  positions[ 3 ] = glm::vec3( 0.637561, 0.69867, -21.4645 );
  positions[ 4 ] = glm::vec3( 265, -16, 0.0 );
  positions[ 5 ] = glm::vec3( 202, -8, 0.0 );
  positions[ 6 ] = glm::vec3( 122, -8, 0.0 );
  positions[ 7 ] = glm::vec3( 40, -7, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 15
  positions[ 0 ] = glm::vec3( 0.637561, 0.69867, -21.4645 );
  positions[ 1 ] = glm::vec3( -1.01163, 0.953711, -20.5014 );
  positions[ 2 ] = glm::vec3( -0.111875, 1.28729, -18.8303 );
  positions[ 3 ] = glm::vec3( -2.0425, 1.4059, -17.5954 );
  positions[ 4 ] = glm::vec3( 40, -7, 0.0 );
  positions[ 5 ] = glm::vec3( -7, -10, 0.0 );
  positions[ 6 ] = glm::vec3( -134, -8, 0.0 );
  positions[ 7 ] = glm::vec3( -93, -10, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 16
  positions[ 0 ] = glm::vec3( -2.0425, 1.4059, -17.5954 );
  positions[ 1 ] = glm::vec3( -0.0562655, 1.41117, -17.4706 );
  positions[ 2 ] = glm::vec3( 0.609171, 1.50025, -18.07 );
  positions[ 3 ] = glm::vec3( 3.08035, 1.1293, -18.6672 );
  positions[ 4 ] = glm::vec3( -93, -10, 0.0 );
  positions[ 5 ] = glm::vec3( -125, -3, 0.0 );
  positions[ 6 ] = glm::vec3( -47, -3, 0.0 );
  positions[ 7 ] = glm::vec3( 0.15, -3.4, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 17
  positions[ 0 ] = glm::vec3( 3.08035, 1.1293, -18.6672 );
  positions[ 1 ] = glm::vec3( 5.69629, 0.953271, -20.3215 );
  positions[ 2 ] = glm::vec3( 8.68158, 0.8150905, -17.075 );
  positions[ 3 ] = glm::vec3( 11.6707, 0.949491, -18.7175 );
  positions[ 4 ] = glm::vec3( 0.15, -3.4, 0.0 );
  positions[ 5 ] = glm::vec3( 1, -2.6, 0.0 );
  positions[ 6 ] = glm::vec3( -1, 3.6, 0.0 );
  positions[ 7 ] = glm::vec3( 0.7, -2, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 18
  positions[ 0 ] = glm::vec3( 11.6707, 0.949491, -18.7175 );
  positions[ 1 ] = glm::vec3( 13.8048, 0.72433, -17.0971 );
  positions[ 2 ] = glm::vec3( 14.4597, 0.52959, -18.3816 );
  positions[ 3 ] = glm::vec3( 14.8484, 0.517183, -18.6426 );
  positions[ 4 ] = glm::vec3( 0.7, -2, 0.0 );
  positions[ 5 ] = glm::vec3( -44, -6, 0.0 );
  positions[ 6 ] = glm::vec3( -36, -1, 0.0 );
  positions[ 7 ] = glm::vec3( -3, -4.0, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 19
  positions[ 0 ] = glm::vec3( 14.8484, 0.517183, -18.6426 );
  positions[ 1 ] = glm::vec3( 17.2874, 0.449192, -17.6789 );
  positions[ 2 ] = glm::vec3( 19.2188, 0.459649, -19.5877 );
  positions[ 3 ] = glm::vec3( 18.1619, 1.04869, -20.0262 );
  positions[ 4 ] = glm::vec3( -3, -4.0, 0.0 );
  positions[ 5 ] = glm::vec3( -39, -3, 0.0 );
  positions[ 6 ] = glm::vec3( -89, 5, 0.0 );
  positions[ 7 ] = glm::vec3( -167, -10, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 20
  positions[ 0 ] = glm::vec3( 18.1619, 1.04869, -20.0262 );
  positions[ 1 ] = glm::vec3( 19.3648, 1.06384, -22.2556 );
  positions[ 2 ] = glm::vec3( 14.7836, 1.25606, -22.5152 );
  positions[ 3 ] = glm::vec3( 16.8598, 1.05984, -19.0605 );
  positions[ 4 ] = glm::vec3( -167, -10, 0.0 );
  positions[ 5 ] = glm::vec3( -257, -15, 0.0 );
  positions[ 6 ] = glm::vec3( -283, -13, 0.0 );
  positions[ 7 ] = glm::vec3( -433, -14, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 21
  positions[ 0 ] = glm::vec3( 16.8598, 1.05984, -19.0605 );
  positions[ 1 ] = glm::vec3( 17.3971, 1.01798, -18.5157 );
  positions[ 2 ] = glm::vec3( 18.4214, 1.17597, -17.7914 );
  positions[ 3 ] = glm::vec3( 18.9106, 1.22371, -17.2851 );
  positions[ 4 ] = glm::vec3( -433, -14, 0.0 );
  positions[ 5 ] = glm::vec3( -413, -4, 0.0 );
  positions[ 6 ] = glm::vec3( -390, -13, 0.0 );
  positions[ 7 ] = glm::vec3( -415, -39, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 22
  positions[ 0 ] = glm::vec3( 18.9106, 1.22371, -17.2851 );
  positions[ 1 ] = glm::vec3( 18.2559, 1.8927, -17.1007 );
  positions[ 2 ] = glm::vec3( 18.0187, 2.50261, -18.6941 );
  positions[ 3 ] = glm::vec3( 18.4589, 2.28536, -18.8299 );
  positions[ 4 ] = glm::vec3( -415, -39, 0.0 );
  positions[ 5 ] = glm::vec3( -390, -36, 0.0 );
  positions[ 6 ] = glm::vec3( -336, -35, 0.0 );
  positions[ 7 ] = glm::vec3( -329, -19, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 23
  positions[ 0 ] = glm::vec3( 18.4589, 2.28536, -18.8299 );
  positions[ 1 ] = glm::vec3( 18.6226, 2.15553, -17.9493 );
  positions[ 2 ] = glm::vec3( 18.6091, 2.17789, -17.4632 );
  positions[ 3 ] = glm::vec3( 18.9847, 2.36555, -16.6756 );
  positions[ 4 ] = glm::vec3( -329, -19, 0.0 );
  positions[ 5 ] = glm::vec3( -369, -13, 0.0 );
  positions[ 6 ] = glm::vec3( -380, -18, 0.0 );
  positions[ 7 ] = glm::vec3( -424, -20, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 24
  positions[ 0 ] = glm::vec3( 18.9847, 2.36555, -16.6756 );
  positions[ 1 ] = glm::vec3( 18.7961, 1.43599, -16.9194 );
  positions[ 2 ] = glm::vec3( 18.7363, 1.2712, -16.8214 );
  positions[ 3 ] = glm::vec3( 18.4916, 1.16388, -16.6048 );
  positions[ 4 ] = glm::vec3( -424, -20, 0.0 );
  positions[ 5 ] = glm::vec3( -327, -9, 0.0 );
  positions[ 6 ] = glm::vec3( -265, -6, 0.0 );
  positions[ 7 ] = glm::vec3( -219, -16, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 25
  positions[ 0 ] = glm::vec3( 18.4916, 1.16388, -16.6048 );
  positions[ 1 ] = glm::vec3( 17.8073, 1.24575, -16.8689 );
  positions[ 2 ] = glm::vec3( 17.3981, 1.24485, -16.7843 );
  positions[ 3 ] = glm::vec3( 17.4465, 1.23574, -16.0101 );
  positions[ 4 ] = glm::vec3( -219, -16, 0.0 );
  positions[ 5 ] = glm::vec3( -274, -24, 0.0 );
  positions[ 6 ] = glm::vec3( -316, -47, 0.0 );
  positions[ 7 ] = glm::vec3( -400, -27, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 26
  positions[ 0 ] = glm::vec3( 17.4465, 1.23574, -16.0101 );
  positions[ 1 ] = glm::vec3( 17.6593, 1.2937, -15.2986 );
  positions[ 2 ] = glm::vec3( 16.5423, 1.16197, -15.2902 );
  positions[ 3 ] = glm::vec3( 16.1436, 1.04587, -15.8602 );
  positions[ 4 ] = glm::vec3( -400, -27, 0.0 );
  positions[ 5 ] = glm::vec3( -452, -16, 0.0 );
  positions[ 6 ] = glm::vec3( -445, -23, 0.0 );
  positions[ 7 ] = glm::vec3( -422, -38, 0.0 );
  _demo_bezier_data.push_back( positions );

  // 27
  positions[ 0 ] = glm::vec3( 16.1436, 1.04587, -15.8602 );
  positions[ 1 ] = glm::vec3( 15.2091, 1.1698, -15.6634 );
  positions[ 2 ] = glm::vec3( 15.7136, 1.20292, -17.64 );
  positions[ 3 ] = glm::vec3( 16.7033, 1.15922, -16.0628 );
  positions[ 4 ] = glm::vec3( -422, -38, 0.0 );
  positions[ 5 ] = glm::vec3( -366, -31, 0.0 );
  positions[ 6 ] = glm::vec3( -300, -23, 0.0 );
  positions[ 7 ] = glm::vec3( -163, -42, 0.0 );
  _demo_bezier_data.push_back( positions );
}

float Camera::BezierCalculation( float A,
                                 float B,
                                 float C,
                                 float D,
                                 float t )
{
  double s   = 1 - t;
  double AB  = A * s + B * t;
  double BC  = B * s + C * t;
  double CD  = C * s + D * t;
  double ABC = AB * s + CD * t;
  double BCD = BC * s + CD * t;
  return ABC * s  + BCD * t;
}

void Camera::DemoScript( float iDeltaTime )
{
  // current speed calculation
  if( iDeltaTime < 1.0 )
  {
    float total_time = 2.0;
    if( _current_speed < _bezier_speed && _current_time < total_time )
    {
      float temp = ( _current_time / total_time );
      _current_speed += iDeltaTime * ( temp * temp ) * 0.3;
    }

    _current_time += iDeltaTime;
  }
  
  // acc calculation
  if( _bezier_acc < 1.0 )
  { 
    if( iDeltaTime < 1.0 )
    {
      _bezier_acc += iDeltaTime * ( _current_speed * 1.7 );
    }
  }
  else
  { 
    if( _bezier_step < _demo_bezier_data.size() - 1 )
    {
      _bezier_step += 1;
      _bezier_acc    = 0.0;
      _current_time  = 0.0;
      _current_speed = 0.01;
    }
  }

  switch( _bezier_step )
  {
    case 0:
      _bezier_speed  = 0.12; 
      if( _bezier_acc < 0.1 )
      {
        _scene->_lights = _scene->_room1_lights;
      }
      break;

    case 1:
      _bezier_speed  = 0.1; 
      if( _bezier_acc > 0.9 )
      {
        _scene->_revolving_door_open = true;
      }
      if( !Mix_PlayingMusic() )
      {
        Mix_FadeInMusic( _scene->_window->_main_music, 1, 5000 );  
      }
      break;

    case 2:
      _bezier_speed  = 0.08; 
      break;

    case 3:
      _bezier_speed  = 0.06; 
      break;

    case 4:
      _bezier_speed  = 0.09; 
      break;

    case 5:
      _bezier_speed  = 0.08; 
      break;

    case 8:
      _bezier_speed  = 0.085; 
      break;

    case 9:
      _bezier_speed  = 0.1; 
      if( _bezier_acc > 0.9 )
      {
        _scene->_simple_door_open    = true;
        _scene->_revolving_door_open = false;
        if( _scene->_current_room == 1 )
        {
          _scene->_current_room = 2;
          _scene->_lights = _scene->_room2_lights; 
        }
      }
      break;

    case 10:
      _bezier_speed  = 0.1; 
      if( _bezier_acc > 0.8 )
      {
        _scene->_revolving_door_open = true;
        _scene->_simple_door_open    = false;
      }
      break;

    case 12:
      _bezier_speed  = 0.075; 
      break;

    case 14:
      _bezier_speed  = 0.08; 
      break;

    case 16:
      _bezier_speed  = 0.1; 
      if( _bezier_acc > 0.95 )
      {
        _scene->_simple_door_open    = true;
        _scene->_revolving_door_open = false;
        if( _scene->_current_room == 2 )
        {
          _scene->_current_room = 3;
          _scene->_lights = _scene->_room3_lights; 
        }
      }
      break;

    case 17:
      _bezier_speed  = 0.07; 
      if( _bezier_acc > 0.85 )
      {
        _scene->_revolving_door_open = true;
        _scene->_simple_door_open    = false;
      }
      break;

    case 18:
      _bezier_speed  = 0.18; 
      break;

    default:
      _bezier_speed  = 0.1; 
      break;
  }

  _position.x = BezierCalculation( _demo_bezier_data[ _bezier_step ][ 0 ].x,
                                   _demo_bezier_data[ _bezier_step ][ 1 ].x,
                                   _demo_bezier_data[ _bezier_step ][ 2 ].x,
                                   _demo_bezier_data[ _bezier_step ][ 3 ].x, 
                                   _bezier_acc );
  _position.y = BezierCalculation( _demo_bezier_data[ _bezier_step ][ 0 ].y,
                                   _demo_bezier_data[ _bezier_step ][ 1 ].y,
                                   _demo_bezier_data[ _bezier_step ][ 2 ].y,
                                   _demo_bezier_data[ _bezier_step ][ 3 ].y, 
                                   _bezier_acc );
  _position.z = BezierCalculation( _demo_bezier_data[ _bezier_step ][ 0 ].z,
                                   _demo_bezier_data[ _bezier_step ][ 1 ].z,
                                   _demo_bezier_data[ _bezier_step ][ 2 ].z,
                                   _demo_bezier_data[ _bezier_step ][ 3 ].z, 
                                   _bezier_acc );

  _yaw = BezierCalculation( _demo_bezier_data[ _bezier_step ][ 4 ].x,
                            _demo_bezier_data[ _bezier_step ][ 5 ].x,
                            _demo_bezier_data[ _bezier_step ][ 6 ].x,
                            _demo_bezier_data[ _bezier_step ][ 7 ].x, 
                            _bezier_acc );

  _pitch = BezierCalculation( _demo_bezier_data[ _bezier_step ][ 4 ].y,
                              _demo_bezier_data[ _bezier_step ][ 5 ].y,
                              _demo_bezier_data[ _bezier_step ][ 6 ].y,
                              _demo_bezier_data[ _bezier_step ][ 7 ].y, 
                              _bezier_acc );

  FrontUpdate();
}

void Camera::KeyboardPositionUpdate( float iDeltaTime )
{
  if( _Z_state == 1 )
  {
    _position -= ( _move_speed * - iDeltaTime ) * _front;
  }

  if( _S_state == 1 )
  {
    _position += ( _move_speed * - iDeltaTime ) * _front;
  }   

  if( _Q_state == 1 )
  {
    _position += glm::normalize( glm::cross( _front, _up ) ) * ( _move_speed * - iDeltaTime );
  }

  if( _D_state == 1 )
  {
    _position -= glm::normalize( glm::cross( _front, _up ) ) * ( _move_speed * - iDeltaTime ); 
  }
}

void Camera::MouseUpdate()
{
  int x,y;                      
  SDL_GetRelativeMouseState( &x, &y );

  float xoffset = ( float )x;
  float yoffset = ( float )y * -1.0; // Reversed since y-coordinates go from bottom to left

  float sensitivity = 0.12;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  _yaw   += xoffset;
  _pitch += yoffset;

  if( _pitch > 89.0f )
  {
    _pitch = 89.0f;
  }
  if( _pitch < -89.0f )
  {
    _pitch = -89.0f;
  }

  FrontUpdate();
}

void Camera::FrontUpdate()
{
  _front.x = cos( glm::radians( _yaw ) ) * cos( glm::radians( _pitch ) );
  _front.y = sin( glm::radians( _pitch ) );
  _front.z = sin( glm::radians( _yaw ) ) * cos( glm::radians( _pitch ) );
  _front = glm::normalize( _front );
}

void Camera::SetProjectionMatrix( glm::mat4 * iProjectionMatrix )
{
  _projection_matrix = *iProjectionMatrix;
}

void Camera::UpdateViewMatrix()
{
  _view_matrix = glm::lookAt( _position, _position + _front, _up );   
}

void Camera::PrintState()
{ 
  std::cout << "\nCamera Status :" << std::endl 
            <<   "---------------" << std::endl;
  std::cout << "Position vector = ( " << _position.x << ", " << _position.y << ", " << _position.z << " )" << std::endl;  
  std::cout << "Front vector    = ( " << _front.x    << ", " << _front.y    << ", " << _front.z    << " )" << std::endl;  
  std::cout << "Yaw   = " << _yaw << std::endl 
            << "Pitch = " << _pitch << std::endl;  
  std::cout << std::endl;
}