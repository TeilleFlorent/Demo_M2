#include "object.hpp"


//******************************************************************************
//**********  Class Object  *****************************************************
//******************************************************************************

Object::Object()
{
}

Object::Object( int       iID,
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
                float     iEmissiveFactor )
{
  _id                  = iID;
  _model_matrix        = iModelMatrix;
  _position            = iPosition;
  _IBL_position        = iIBLPosition;
  _angle               = iAngle;
  _scale               = iScale;
  _uv_scale            = iUvScale;
  _alpha               = iAlpha;
  _generate_shadow     = iGenerateShadow;
  _receiv_shadow       = iReceivShadow;
  _shadow_darkness     = iShadowDarkness;
  _shadow_bias         = iShadowBias;
  _bloom               = iBloom;
  _bloom_brightness    = iBloomBrightness;
  _opacity_map         = iOpacityMap;
  _normal_map          = iNormalMap;
  _height_map          = iHeightMap;
  _displacement_factor = iDisplacementFactor;
  _tessellation_factor = iTessellationFactor;
  _material_id         = iMaterialID;
  _emissive            = iEmissive;
  _emissive_factor     = iEmissiveFactor;
}

void Object::Set( Object iSourceObject )
{
  _id                  = iSourceObject._id; 
  _model_matrix        = iSourceObject._model_matrix;                 
  _position            = iSourceObject._position;    
  _IBL_position        = iSourceObject._IBL_position;       
  _angle               = iSourceObject._angle;               
  _scale               = iSourceObject._scale;               
  _uv_scale            = iSourceObject._uv_scale;            
  _alpha               = iSourceObject._alpha;               
  _generate_shadow     = iSourceObject._generate_shadow;     
  _receiv_shadow       = iSourceObject._receiv_shadow;      
  _shadow_darkness     = iSourceObject._shadow_darkness;
  _shadow_bias         = iSourceObject._shadow_bias;     
  _bloom               = iSourceObject._bloom;               
  _bloom_brightness    = iSourceObject._bloom_brightness;    
  _opacity_map         = iSourceObject._opacity_map;         
  _normal_map          = iSourceObject._normal_map;          
  _height_map          = iSourceObject._height_map;          
  _displacement_factor = iSourceObject._displacement_factor; 
  _tessellation_factor = iSourceObject._tessellation_factor; 
  _material_id         = iSourceObject._material_id; 
  _emissive            = iSourceObject._emissive;
  _emissive_factor     = iSourceObject._emissive_factor; 
}


