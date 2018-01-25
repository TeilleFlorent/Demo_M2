#include "object.hpp"


//******************************************************************************
//**********  Class Object  *****************************************************
//******************************************************************************

Object::Object( int       iID,
                glm::vec3 iPosition,
                float     iAngle,
                glm::vec3 iScale,
                float     iAlpha,
                bool      iGenerateShadow,
                bool      iReceivShadow,
                float     iShadowDarkness,
                float     iBloom,
                float     iBloomBrightness,
                float     iOpacityMap )
{
  _id               = iID;
  _position         = iPosition;
  _angle            = iAngle;
  _scale            = iScale;
  _alpha            = iAlpha;
  _generate_shadow  = iGenerateShadow;
  _receiv_shadow    = iReceivShadow;
  _shadow_darkness  = iShadowDarkness;
  _bloom            = iBloom;
  _bloom_brightness = iBloomBrightness;
  _opacity_map      = iOpacityMap;
}

