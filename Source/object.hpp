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
            float     iAcca,
            glm::vec3 iScale,
            float     iAlpha,
            int       iShiniStr,
            bool      iGenerateShadow,
            float     iShadowDarkness,
            bool      iNormalMapping,
            bool      iBloom,
            float     iBloomBrightness );


    // Object class members
    // -------------------
    
    float     _id;
    glm::vec3 _position;
    float     _angle;
    float     _acca;
    glm::vec3 _scale;
    float     _alpha;
    int       _shini_str;
    bool      _generate_shadow;
    float     _shadow_darkness;
    bool      _normal_mapping;
    bool      _bloom;
    float     _bloom_brightness;
};

#endif  // OBJECT_H
