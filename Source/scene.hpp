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

    void IBLInitialization();

    void TesselationInitialization();

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
    Shader _forward_displacement_pbr_shader;
    Shader _skybox_shader;
    Shader _flat_color_shader;
    Shader _observer_shader;
    Shader _blur_shader;
    Shader _post_process_shader;
    Shader _MS_blit_shader;
    Shader _cube_map_converter_shader;
    Shader _diffuse_irradiance_shader;
    Shader _specular_pre_filter_shader;
    Shader _specular_pre_brdf_shader;

    Shader _geometry_pass_shader;
    Shader _lighting_pass_shader;
    Shader _empty_shader;

    // VAOs
    unsigned int _ground_VAO;

    // VBOs
    unsigned int _ground_VBO;

    // IBOs
    unsigned int _ground_IBO;
    std::vector< unsigned int > _ground_indices;

    // Textures
    int _current_env;
    std::vector< unsigned int > _hdr_textures;
    std::vector< unsigned int > _env_cubeMaps;
    std::vector< unsigned int > _irradiance_cubeMaps;
    std::vector< unsigned int > _pre_filter_cubeMaps;
    unsigned int _pre_brdf_texture;
  
    unsigned int _tex_albedo_ground;
    unsigned int _tex_normal_ground;
    unsigned int _tex_height_ground;
    unsigned int _tex_AO_ground;
    unsigned int _tex_roughness_ground;
    unsigned int _tex_metalness_ground;

    std::vector< const GLchar * > _faces; // skybox textures path

    // Deferred rendering data
    unsigned int _g_buffer_FBO;
    std::vector< unsigned int > _g_buffer_textures; // [ position, normal, color, roughness_metalness_AO, depth, lighting, brightest ]

    // Bloom parameters
    float _exposure;
    bool  _bloom;
    float _blur_downsample;
    int   _blur_pass_count;
    float _blur_offset_factor;

    // Multi sample parameters
    bool _multi_sample;
    int  _nb_multi_sample;

    // IBL parameters
    int   _res_env_cubeMap;

    int   _res_irradiance_cubeMap;
    float _irradiance_sample_delta;

    int          _res_pre_filter_cubeMap;
    unsigned int _pre_filter_sample_count;
    unsigned int _pre_filter_max_mip_Level;

    unsigned int _res_pre_brdf_texture;
    unsigned int _pre_brdf_sample_count;

    // Tessellation parameters
    int _tess_max_patch_vertices;
    int _tess_patch_vertices_count;

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
    Object *              _ground1;
    Object *              _ink_bottle;
    Object *              _collection_car;

    // Models
    Model * _table_model;
    Model * _sphere_model;
    Model * _ink_bottle_model;
    Model * _collection_car_model;
};

#endif  // SCENE_H
