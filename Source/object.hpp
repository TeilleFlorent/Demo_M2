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
            float     iBloom,
            float     iBloomBrightness,
            float     iOpacityMap );


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
    float     _bloom;
    float     _bloom_brightness;
    float     _opacity_map;
};

#endif  // OBJECT_H
