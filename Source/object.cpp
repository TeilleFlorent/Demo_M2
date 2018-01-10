#include "object.hpp"


//******************************************************************************
//**********  Class Object  *****************************************************
//******************************************************************************

Object::Object( int       iID,
                glm::vec3 iPosition,
                float     iAngle,
                float     iAngleAcc,
                glm::vec3 iScale,
                float     iAlpha,
                int       iShiniStr,
                bool      iGenerateShadow,
                bool      iReceivShadow,
                float     iShadowDarkness,
                float     iBloom,
                float     iBloomBrightness )
{
  _id               = iID;
  _position         = iPosition;
  _angle            = iAngle;
  _angle_acc        = iAngleAcc;
  _scale            = iScale;
  _alpha            = iAlpha;
  _shini_str        = iShiniStr;
  _generate_shadow  = iGenerateShadow;
  _receiv_shadow    = iReceivShadow;
  _shadow_darkness  = iShadowDarkness;
  _bloom            = iBloom;
  _bloom_brightness = iBloomBrightness;
}

