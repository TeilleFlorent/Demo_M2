#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include <glm/glm.hpp>

using namespace std;


//******************************************************************************
//**********  Class Point Light  ***********************************************
//******************************************************************************

class PointLight
{

  public:


    // PointLight functions
    // --------------------

    PointLight( glm::vec3 iPosition,
                glm::vec3 iColor,
                float     iIntensity,
                float     iMaxLightingDistance );

    static void SetLightsMultiplier( float iMultiplier );

    static float GetLightsMultiplier();

    void UpdateBoundingSphereScale();


    // PointLight class members
    // ------------------------
    
    glm::vec3 _position;  
    glm::vec3 _color;
    float     _intensity;
    float     _max_lighting_distance;


  private:

    static float _intensity_multiplier;

};

#endif  // POINT_LIGHT_H
