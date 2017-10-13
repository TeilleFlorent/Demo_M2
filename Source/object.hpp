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
            float     iScale,
            float     iAlpha,
            float     iAmbientStr,
            float     iDiffuseStr,
            float     iSpecularStr,
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
    float     _scale;
    float     _alpha;
    float     _ambient_str;
    float     _diffuse_str;
    float     _specular_str;
    int       _shini_str;
    bool      _generate_shadow;
    float     _shadow_darkness;
    bool      _normal_mapping;
    bool      _bloom;
    float     _bloom_brightness;
};

#endif  // OBJECT_H
