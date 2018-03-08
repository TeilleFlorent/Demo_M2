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
    Scene();

    Scene( Window * iParentWindow );

    void Quit();

    void SceneDataInitialization();

    void ShadersInitialization();
    
    void LightsInitialization();

    void ObjectsInitialization();

    void IBLInitialization();

    void TesselationInitialization();

    void ModelsLoading();

    void ObjectsIBLInitialization();

    void ObjectCubemapsGeneration( Object *     iObject,
                                   bool         iNeedAllWalls,
                                   unsigned int iWallID );

    void DeferredBuffersInitialization();

    void SceneDepthPass();

    void SceneForwardRendering();

    void DeferredGeometryPass( glm::mat4 * iProjectionMatrix,
                               glm::mat4 * iViewMatrix );

    void DeferredLightingPass( glm::mat4 * iProjectionMatrix,
                               glm::mat4 * iViewMatrix );

    void SceneDeferredRendering();

    void BlurProcess();

    void PostProcess();

    void AnimationsUpdate();

    void RevolvingDoorScript();

    void SimpleDoorScript();

    void AudioInitialization();

    void LoadAudio();


    // Scene class members
    // -------------------
    
    // Pipeline Type 
    int _pipeline_type;

    // near far
    float _near;
    float _far;
    float _shadow_near;
    float _shadow_far;

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
    Shader _point_shadow_depth_shader;

    Shader _geometry_pass_shader;
    Shader _lighting_pass_shader;
    Shader _empty_shader;

    // VAOs
    unsigned int _ground1_VAO;
    unsigned int _ground2_VAO;
    unsigned int _wall1_VAO;
    unsigned int _wall2_VAO;

    // VBOs
    unsigned int _ground1_VBO;
    unsigned int _ground2_VBO;
    unsigned int _wall1_VBO;
    unsigned int _wall2_VBO;

    // IBOs
    unsigned int                _ground1_IBO;
    std::vector< unsigned int > _ground1_indices;
    unsigned int                _ground2_IBO;
    std::vector< unsigned int > _ground2_indices;
    unsigned int                _wall1_IBO;
    std::vector< unsigned int > _wall1_indices;
    unsigned int                _wall2_IBO;
    std::vector< unsigned int > _wall2_indices;

    // Textures
    unsigned int _pre_brdf_texture;
    
    std::vector< std::vector< unsigned int > > _loaded_materials;
   
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
    int   _res_env_cubemap;

    int   _res_irradiance_cubemap;
    float _irradiance_sample_delta;

    int          _res_pre_filter_cubemap;
    unsigned int _pre_filter_sample_count;
    unsigned int _pre_filter_max_mip_Level;

    unsigned int _res_pre_brdf_texture;
    unsigned int _pre_brdf_sample_count;

    // Tessellation parameters
    int _tess_max_patch_vertices;
    int _tess_patch_vertices_count;

    // Omnidirectional shadow mapping parameters
    unsigned int _depth_cubemap_res;

    // Pointer on the scene window
    Window * _window;

    // Clock
    Clock * _clock;

    // Camera
    Camera * _camera;

    // Point Lights
    std::vector < PointLight >  _lights;
    std::vector < PointLight >  _room1_lights;
    std::vector < PointLight >  _room2_lights;
    std::vector < PointLight >  _room3_lights;
    bool _render_lights_volume;

    // Scene's objects
    std::vector< Object > _walls_type1;
    std::vector< Object > _walls_type2;
    std::vector< Object > _grounds_type1;
    std::vector< Object > _revolving_door;
    std::vector< Object > _simple_door;
    std::vector< Object > _top_light;
    std::vector< Object > _wall_light;
    Object                _ink_bottle;
    Object                _room1_table1;
    Object                _bottle;
    Object                _ball;
    Object                _box_bag;
    Object                _chest;
    Object                _sofa;
    Object                _sack;
    Object                _room1_table2;
    Object                _book;
    Object                _radio;
    Object                _screen;
    Object                _bike;
    Object                _pilar;
    Object                _scanner;
    Object                _room2_table1;
    Object                _mask;
    Object                _arm;
    Object                _tank;
    Object                _shelving;
    Object                _gun1;
    Object                _gun2;
    Object                _gun3;
    Object                _room3_table1;
    Object                _room3_table2;
    Object                _helmet;
    Object                _knife;
    Object                _grenade;
    Object                _gun4;
    Object                _gun5;
    Object                _room3_table3;
    Object                _katana;
    Object                _helmet2;
    float                 _ground_size;
    float                 _wall_size;

    // Models
    Model * _sphere_model;
    Model * _ink_bottle_model;
    Model * _revolving_door_model;
    Model * _simple_door_model;
    Model * _top_light_model;
    Model * _wall_light_model;
    Model * _room1_table1_model;
    Model * _bottle_model;
    Model * _ball_model;
    Model * _box_bag_model;
    Model * _chest_model;
    Model * _sofa_model;
    Model * _sack_model;
    Model * _room1_table2_model;
    Model * _book_model;
    Model * _radio_model;
    Model * _screen_model;
    Model * _bike_model;
    Model * _pilar_model;
    Model * _scanner_model;
    Model * _room2_table1_model;
    Model * _mask_model;
    Model * _arm_model;
    Model * _tank_model;
    Model * _shelving_model;
    Model * _gun1_model;
    Model * _gun2_model;
    Model * _gun3_model;
    Model * _room3_table1_model;
    Model * _room3_table2_model;
    Model * _helmet_model;
    Model * _knife_model;
    Model * _grenade_model;
    Model * _gun4_model;
    Model * _gun5_model;
    Model * _room3_table3_model;
    Model * _katana_model;
    Model * _helmet2_model;

    // Revolving door rotation matrix
    glm::mat4 _door_rotation_matrix;
    glm::mat4 _door1_rotation_matrix;
    glm::mat4 _door2_rotation_matrix;
    float     _door_angle;
    bool      _revolving_door_open;

    // Simple door translation matrix
    glm::mat4 _door_translation_matrix1;
    glm::mat4 _door_translation_matrix2;
    glm::vec3 _door_position;
    bool _simple_door_open;

    // current room
    int _current_room;
    int _current_shadow_light_source;

    // walls render iterators
    unsigned int _walls_start_it;
    unsigned int _walls_end_it;

    // grounds render iterators
    unsigned int _grounds_start_it;
    unsigned int _grounds_end_it;
};

#endif  // SCENE_H
