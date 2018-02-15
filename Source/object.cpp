#include "object.hpp"


//******************************************************************************
//**********  Class Object  *****************************************************
//******************************************************************************

Object::Object( int       iID,
                glm::vec3 iPosition,
                float     iAngle,
                glm::vec3 iScale,
                glm::vec2 iUvScale,
                float     iAlpha,
                bool      iGenerateShadow,
                bool      iReceivShadow,
                float     iShadowDarkness,
                bool      iBloom,
                float     iBloomBrightness,
                bool      iOpacityMap,
                bool      iNormalMap,
                bool      iHeightMap,
                float     iDisplacementFactor,
                float     iTessellationFactor )
{
  _id                  = iID;
  _position            = iPosition;
  _angle               = iAngle;
  _scale               = iScale;
  _uv_scale            = iUvScale;
  _alpha               = iAlpha;
  _generate_shadow     = iGenerateShadow;
  _receiv_shadow       = iReceivShadow;
  _shadow_darkness     = iShadowDarkness;
  _bloom               = iBloom;
  _bloom_brightness    = iBloomBrightness;
  _opacity_map         = iOpacityMap;
  _normal_map          = iNormalMap;
  _height_map          = iHeightMap;
  _displacement_factor = iDisplacementFactor;
  _tessellation_factor = iTessellationFactor;
}

