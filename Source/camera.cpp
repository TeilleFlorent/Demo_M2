#include "camera.hpp"


//******************************************************************************
//**********  Class Camera  ****************************************************
//******************************************************************************

Camera::Camera()
{
  _position = glm::vec3( -7.02172, 2.42351, -6.27063 );
  _front    = glm::vec3( 0.724498, -0.409127, 0.554724 );
  _up       = glm::vec3( 0.0f, 1.0f,  0.0f );     
    
  _near       = 0.01;
  _far        = 100.0;
  _yaw        = 37.44;
  _pitch      = -24.0;

  _move_speed = 3.0;

  _Z_state = 0;
  _D_state = 0;
  _Q_state = 0;
  _S_state = 0;
}

void Camera::CameraUpdate( float iDeltaTime )
{
  KeyboardPositionUpdate( iDeltaTime );
  MouseFrontUpdate();
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

void Camera::MouseFrontUpdate()
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

  _front.x = cos( glm::radians( _yaw ) ) * cos( glm::radians( _pitch ) );
  _front.y = sin( glm::radians( _pitch ) );
  _front.z = sin( glm::radians( _yaw ) ) * cos( glm::radians( _pitch ) );
  _front = glm::normalize( _front );
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