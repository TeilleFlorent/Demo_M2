#include "point_light.hpp"


//******************************************************************************
//**********  Class PointLight  ************************************************
//******************************************************************************

float PointLight::_intensity_multiplier; 

PointLight::PointLight( glm::vec3 iPosition,
                        glm::vec3 iColor,
                        float     iIntensity,
                        float     iAttenConstant,
                        float     iAttenLinear,
                        float     iAttenExp )
{
  _position             = iPosition;
  _color                = iColor;
  _intensity            = iIntensity;
  _attenuation_constant = iAttenConstant;
  _attenuation_linear   = iAttenLinear;
  _attenuation_exp      = iAttenExp;
}

void PointLight::SetLightsMultiplier( float iMultiplier )
{
  _intensity_multiplier = iMultiplier;
}

float PointLight::GetLightsMultiplier()
{
  return _intensity_multiplier;
}


