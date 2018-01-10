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
            float     iAngleAcc,
            glm::vec3 iScale,
            float     iAlpha,
            int       iShiniStr,
            bool      iGenerateShadow,
            bool      iReceivShadow,
            float     iShadowDarkness,
            float     iBloom,
            float     iBloomBrightness );


    // Object class members
    // -------------------
    
    float     _id;
    glm::vec3 _position;
    float     _angle;
    float     _angle_acc;
    glm::vec3 _scale;
    float     _alpha;
    int       _shini_str;
    bool      _generate_shadow;
    bool      _receiv_shadow;
    float     _shadow_darkness;
    float     _bloom;
    float     _bloom_brightness;
};

#endif  // OBJECT_H
