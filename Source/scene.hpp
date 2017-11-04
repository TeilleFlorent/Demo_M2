#include "shader.hpp"
#include "clock.hpp"
#include "light.hpp"
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

    void SceneForwardRendering( bool iIsFinalFBO );

    void DeferredGeometryPass();

    void DeferredLightingPass();

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
    Shader _blit_shader;
    Shader _cube_map_converter_shader;
    Shader _diffuse_irradiance_shader;
    Shader _geometry_pass_shader;
    Shader _lighting_pass_shader;

    // VAOs
    GLuint _lampVAO;
    GLuint _groundVAO;

    // VBOs
    GLuint _lampVBO;
    GLuint _groundVBO;

    // Textures
    std::vector< unsigned int > _hdr_textures;
    std::vector< unsigned int > _env_cubemaps;
    std::vector< unsigned int > _irradiance_maps;   
    int _current_env;
   
    std::vector< const GLchar * > _faces; // data skybox cube map texture

    GLuint _tex_albedo_ground;
    GLuint _tex_normal_ground;
    GLuint _tex_height_ground;
    GLuint _tex_AO_ground;
    GLuint _tex_roughness_ground;
    GLuint _tex_metalness_ground;

    // Deferred rendering data
    unsigned int _g_buffer_FBO;
    unsigned int _g_buffer_RBO;
    std::vector< unsigned int > _g_buffer_textures; // [ position, normal, color, roughness_metalness_AO ]

    // Bloom param
    float _exposure;
    bool  _bloom;
    float _bloom_downsample;

    // Multi sample param
    bool _multi_sample;
    int  _nb_multi_sample;

    // IBL param
    int   _res_IBL_cubeMap;
    int   _res_irradiance_cubeMap;
    float _irradiance_sample_delta;

    // Pointer on the scene window
    Window * _window;

    // Clock
    Clock * _clock;

    // Camera
    Camera * _camera;

    // Lights
    std::vector < Light > _lights;

    // Scene's objects
    std::vector< Object > _tables;
    Object * _ground1;

    // Models
    Model * _table_model;
};

#endif  // SCENE_H
