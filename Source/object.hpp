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
            float     iTessellationFactor );


    // Object class members
    // -------------------
    
    float     _id;
    glm::vec3 _position;
    float     _angle;
    glm::vec3 _scale;
    glm::vec2 _uv_scale;
    float     _alpha;
    bool      _generate_shadow;
    bool      _receiv_shadow;
    float     _shadow_darkness;
    bool      _bloom;
    float     _bloom_brightness;
    bool      _opacity_map;
    bool      _normal_map;
    bool      _height_map;
    float     _displacement_factor;
    float     _tessellation_factor;
};

#endif  // OBJECT_H
