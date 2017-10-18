#include "light.hpp"


//******************************************************************************
//**********  Class Light  *****************************************************
//******************************************************************************

float Light::_intensity_multiplier; 

Light::Light( glm::vec3 iPosition,
              glm::vec3 iColor,
              float iIntensity )
{
  _position  = iPosition;
  _color     = iColor;
  _intensity = iIntensity;
}

void Light::SetLightsMultiplier( float iMultiplier )
{
  _intensity_multiplier = iMultiplier;
}

float Light::GetLightsMultiplier()
{
  return _intensity_multiplier;
}


