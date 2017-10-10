#include "light.hpp"


//******************************************************************************
//**********  Class Light  *****************************************************
//******************************************************************************

Light::Light( glm::vec3 iPosition,
              glm::vec3 iColor,
              float iIntensity )
{
  _position  = iPosition;
  _color     = iColor;
  _intensity = iIntensity;
}
