#include "point_light.hpp"


//******************************************************************************
//**********  Class PointLight  ************************************************
//******************************************************************************

float PointLight::_intensity_multiplier; 

PointLight::PointLight( glm::vec3 iPosition,
                        glm::vec3 iColor,
                        float     iIntensity,
                        float     iMaxLightingDistance )
{
  _position              = iPosition;
  _color                 = iColor;
  _intensity             = iIntensity;
  _max_lighting_distance = iMaxLightingDistance;
}

void PointLight::SetLightsMultiplier( float iMultiplier )
{
  _intensity_multiplier = iMultiplier;
}

float PointLight::GetLightsMultiplier()
{
  return _intensity_multiplier;
}