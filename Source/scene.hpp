#include "shader.hpp"
#include "clock.hpp"
#include "light.hpp"
#include "object.hpp"
#include "classic_model.hpp"
#include "camera.hpp"


#ifndef SCENE_H
#define SCENE_H

#include <GL/glew.h>


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

    void LoadModels();

    void RenderScene( bool iIsFinalFBO );

    void BlurProcess();

    void BloomProcess();


    // Scene class members
    // -------------------
    
    // Shaders
    Shader _pbr_shader;
    Shader _skybox_shader;
    Shader _flat_color_shader;
    Shader _observer_shader;
    Shader _blur_shader;
    Shader _bloom_shader;
    Shader _blit_shader;
    Shader _cube_map_converter_shader;
    Shader _diffuse_irradiance_shader;

    // VAOs
    GLuint _skyboxVAO;
    GLuint _lampVAO;
    GLuint _groundVAO;

    // VBOs
    GLuint _skyboxVBO;
    GLuint _lampVBO;
    GLuint _groundVBO;

    // Textures
    unsigned int _hdrTexture;
    unsigned int _envCubemap;
    unsigned int _irradianceMap;   
    std::vector< const GLchar * > _faces; // data skybox cube map texture

    GLuint _tex_albedo_ground;
    GLuint _tex_normal_ground;
    GLuint _tex_height_ground;
    GLuint _tex_AO_ground;
    GLuint _tex_roughness_ground;
    GLuint _tex_metalness_ground;

    // Bloom param
    float _exposure;
    bool _bloom;
    float _bloom_downsample;

    // Multi sample param
    bool _multi_sample;
    int _nb_multi_sample;

    // IBL param
    int _res_IBL_cubeMap;
    int _res_irradiance_cubeMap;

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
