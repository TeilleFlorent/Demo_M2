#ifndef OBJECT_H
#define OBJECT_H

#include <glm/glm.hpp>
#include <vector>


//******************************************************************************
//**********  Class Object  ****************************************************
//******************************************************************************

class Object
{

  public:


    // Object functions
    // ----------------
    Object();

    Object( int       iID,
            glm::mat4 iModelMatrix,
            glm::vec3 iPosition,
            glm::vec3 iIBLPosition,
            float     iAngle,
            glm::vec3 iScale,
            glm::vec2 iUvScale,
            float     iAlpha,
            bool      iGenerateShadow,
            bool      iReceivShadow,
            float     iShadowDarkness,
            float     iShadowBias,
            bool      iBloom,
            float     iBloomBrightness,
            bool      iOpacityMap,
            bool      iNormalMap,
            bool      iHeightMap,
            float     iDisplacementFactor,
            float     iTessellationFactor,
            int       iMaterialID,
            bool      iEmissive,
            float     iEmissiveFactor,
            bool      iParallaxCubemap,
            bool      iIBL );

    void Set( Object iSourceObject );


    // Object class members
    // -------------------
    
    float                       _id;
    glm::mat4                   _model_matrix;
    glm::vec3                   _position;
    glm::vec3                   _IBL_position;
    float                       _angle;
    glm::vec3                   _scale;
    glm::vec2                   _uv_scale;
    float                       _alpha;
    bool                        _generate_shadow;
    bool                        _receiv_shadow;
    float                       _shadow_darkness;
    float                       _shadow_bias;
    bool                        _bloom;
    float                       _bloom_brightness;
    bool                        _opacity_map;
    bool                        _normal_map;
    bool                        _height_map;
    float                       _displacement_factor;
    float                       _tessellation_factor;
    int                         _material_id;
    bool                        _emissive;
    float                       _emissive_factor;
    std::vector< unsigned int > _IBL_cubemaps;
    bool                        _parallax_cubemap;
    bool                        _IBL;
};

#endif  // OBJECT_H
