#include "object.hpp"


//******************************************************************************
//**********  Class Object  *****************************************************
//******************************************************************************

Object::Object( int       iID,
                glm::vec3 iPosition,
                float     iAngle,
                float     iAcca,
                glm::vec3 iScale,
                float     iAlpha,
                int       iShiniStr,
                bool      iGenerateShadow,
                float     iShadowDarkness,
                bool      iNormalMapping,
                float     iBloom,
                float     iBloomBrightness )
{
  _id               = iID;
  _position         = iPosition;
  _angle            = iAngle;
  _acca             = iAcca;
  _scale            = iScale;
  _alpha            = iAlpha;
  _shini_str        = iShiniStr;
  _generate_shadow  = iGenerateShadow;
  _shadow_darkness  = iShadowDarkness;
  _normal_mapping   = iNormalMapping;
  _bloom            = iBloom;
  _bloom_brightness = iBloomBrightness;
}

