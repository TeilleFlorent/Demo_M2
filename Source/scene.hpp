#include "shader.hpp"
#include "clock.hpp"
#include "point_light.hpp"
#include "object.hpp"
#include "classic_model.hpp"
#include "camera.hpp"


#ifndef SCENE_H
#define SCENE_H

#include <GL/glew.h>

#define FORWARD_RENDERING 0
#define DEFERRED_RENDERING 1


//******************************************************************************
//**********  Class Scene  *****************************************************
//******************************************************************************

class Window;

class Scene
{

  public:


    // Scene functions
    // ---------------

    Scene( Window * iParentWindow );

    void Quit();

    void SceneDataInitialization();

    void ShadersInitialization();
    
    void LightsInitialization();

    void ObjectsInitialization();

    void IBLCubeMapsInitialization();

    void ModelsLoading();

    void DeferredBuffersInitialization();

    void SceneForwardRendering();

    void DeferredGeometryPass( glm::mat4 * iProjectionMatrix,
                               glm::mat4 * iViewMatrix );

    void DeferredLightingPass( glm::mat4 * iProjectionMatrix,
                               glm::mat4 * iViewMatrix );

    void SceneDeferredRendering();

    void BlurProcess();

    void PostProcess();


    // Scene class members
    // -------------------
    
    // Pipeline Type 
    int _pipeline_type;

    // Shaders
    Shader _forward_pbr_shader;
    Shader _skybox_shader;
    Shader _flat_color_shader;
    Shader _observer_shader;
    Shader _blur_shader;
    Shader _post_process_shader;
    Shader _MS_blit_shader;
    Shader _cube_map_converter_shader;
    Shader _diffuse_irradiance_shader;
    Shader _specular_pre_filter_shader;
    Shader _geometry_pass_shader;
    Shader _lighting_pass_shader;
    Shader _empty_shader;

    // VAOs
    unsigned int _ground_VAO;

    // VBOs
    unsigned int _ground_VBO;

    // Textures
    std::vector< unsigned int > _hdr_textures;
    std::vector< unsigned int > _env_cubeMaps;
    std::vector< unsigned int > _irradiance_cubeMaps;
    std::vector< unsigned int > _pre_filter_cubeMaps;   
    int _current_env;
   
    std::vector< const GLchar * > _faces; // data skybox cube map texture

    unsigned int _tex_albedo_ground;
    unsigned int _tex_normal_ground;
    unsigned int _tex_height_ground;
    unsigned int _tex_AO_ground;
    unsigned int _tex_roughness_ground;
    unsigned int _tex_metalness_ground;

    // Deferred rendering data
    unsigned int _g_buffer_FBO;
    std::vector< unsigned int > _g_buffer_textures; // [ position, normal, color, roughness_metalness_AO, depth, lighting, brightest ]

    // Bloom param
    float _exposure;
    bool  _bloom;
    float _blur_downsample;
    int   _blur_pass_count;
    float _blur_offset_factor;

    // Multi sample param
    bool _multi_sample;
    int  _nb_multi_sample;

    // IBL param
    int   _res_env_cubeMap;
    int   _res_irradiance_cubeMap;
    int   _res_pre_filter_cubeMap;
    float _irradiance_sample_delta;

    // Pointer on the scene window
    Window * _window;

    // Clock
    Clock * _clock;

    // Camera
    Camera * _camera;

    // Point Lights
    std::vector < PointLight > _lights;
    bool _render_lights_volume;

    // Scene's objects
    std::vector< Object > _tables;
    Object * _ground1;

    // Models
    Model * _table_model;
    Model * _sphere_model;
};

#endif  // SCENE_H
