#ifndef OBJECT_H
#define OBJECT_H

#include <glm/glm.hpp>


//******************************************************************************
//**********  Class Object  ****************************************************
//******************************************************************************

class Object
{

  public:


    // Object functions
    // ----------------

    Object( int       iID,
            glm::vec3 iPosition,
            float     iAngle,
            glm::vec3 iScale,
            float     iAlpha,
            bool      iGenerateShadow,
            bool      iReceivShadow,
            float     iShadowDarkness,
            bool      iBloom,
            float     iBloomBrightness,
            bool      iOpacityMap,
            bool      iNormalMap );


    // Object class members
    // -------------------
    
    float     _id;
    glm::vec3 _position;
    float     _angle;
    glm::vec3 _scale;
    float     _alpha;
    bool      _generate_shadow;
    bool      _receiv_shadow;
    float     _shadow_darkness;
    bool      _bloom;
    float     _bloom_brightness;
    bool      _opacity_map;
    bool      _normal_map;
};

#endif  // OBJECT_H
