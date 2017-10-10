#include "object.hpp"


//******************************************************************************
//**********  Class Object  *****************************************************
//******************************************************************************

Object::Object( int       iID,
                glm::vec3 iPosition,
                float     iAngle,
                float     iAcca,
                float     iScale,
                float     iAlpha,
                float     iAmbientStr,
                float     iDiffuseStr,
                float     iSpecularStr,
                int       iShiniStr,
                bool      iGenerateShadow,
                float     iShadowDarkness,
                bool      iNormalMapping )
{
  _id              = iID;
  _position        = iPosition;
  _angle           = iAngle;
  _acca            = iAcca;
  _scale           = iScale;
  _alpha           = iAlpha;
  _ambient_str     = iAmbientStr;
  _diffuse_str     = iDiffuseStr;
  _specular_str    = iSpecularStr;
  _shini_str       = iShiniStr;
  _generate_shadow = iGenerateShadow;
  _shadow_darkness = iShadowDarkness;
  _normal_mapping  = iNormalMapping;
}

