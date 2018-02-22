#include "shader.hpp"


#ifndef CLASSIC_MODEL_H
#define CLASSIC_MODEL_H


#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"
#include "glm/ext.hpp"

#define GLEW_STATIC
#include <GL/glew.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


using namespace std;


//******************************************************************************
//**********  Class Vertex  ****************************************************
//******************************************************************************

class Vertex
{
  public:  
    
    glm::vec3 _position;
    glm::vec3 _normal;
    glm::vec2 _uv;
    glm::vec3 _tangent;
    glm::vec3 _bi_tangent;
};


//******************************************************************************
//**********  Class Texture  ***************************************************
//******************************************************************************

class Texture
{

  public:
  
    unsigned int _id;
    std::string  _type;
    std::string  _path;
};


//******************************************************************************
//**********  Class Mesh  ******************************************************
//******************************************************************************

class Mesh
{

  public:


    // Class functions
    // ---------------
    Mesh( vector< Vertex >  iVertices,
          vector< GLuint >  iIndices,
          vector< Texture > iTextures,
          glm::mat4         iLocalTransform,
          aiString          iMeshName,
          bool              iOpacityMap );

    void Draw( Shader    iShader,
               int       iModelID,
               int       iMeshNumber,
               glm::mat4 iModelMatrix,
               bool      iNormalMap,
               bool      iHeightMap,
               float     iOpacityDiscard );

   void DrawDepth( Shader    iShader,
                   glm::mat4 iModelMatrix ); 

    
    // Class members
    // -------------
    vector< Vertex >  _vertices;
    vector< GLuint >  _indices;
    vector< Texture > _textures;
    glm::mat4         _local_transform;
    aiString          _name;
    bool              _opacity_map;


  private:
  
    unsigned int _VAO, _VBO, _EBO;

    void SetupMesh();
    
};


//******************************************************************************
//**********  Class Model  *****************************************************
//******************************************************************************

class Toolbox;
class Scene;

class Model 
{

  public:


    // Class functions
    // ---------------
    Model( string  iPath,
           int     iID,
           string  iName,
           bool    iNormalMap,
           bool    iHeightMap,
           Scene * iScene );

    void Draw( Shader    iShader,
               glm::mat4 iModelMatrix );   

    void DrawDepth( Shader    iShader,
                    glm::mat4 iModelMatrix );   

    void PrintInfos();

    void LoadModel( string iPath );

    void ProcessNode( aiNode * iNode );

    Mesh ProcessMesh( aiMesh *    iMesh,
                      aiMatrix4x4 iLocalTransform,
                      aiString    iNodeName  );

    vector< Texture > LoadMeshTextures( aiString     iNodeName,
                                        bool *       oOpacityMap,
                                        unsigned int iMaterialIndex );

    Texture LoadTexture( string iTextureType,
                         string iTextureName,
                         int    iInternalFormat,
                         int    iFormat );
    
    unsigned int TextureFromFile( string iTexturePath,
                                  int    iInternalFormat,
                                  int    iFormat );

    static void SetToolbox( Toolbox * iToolbox );

    static Toolbox * GetToolbox();


    // Class members
    // -------------
    int               _model_id;
    string            _model_name;
    string            _directory;

    vector< Mesh >    _meshes;
    int               _vertice_count;

    vector< Texture > _textures_loaded;
    Assimp::Importer  _importer;
    const aiScene *   _assimp_scene;

    // Pointer on the scene where this model is render
    Scene *           _scene;

    bool _normal_map;
    bool _height_map;

  private:

    // Pointer on the window program toolbox
    static Toolbox * _toolbox;

};    

#endif  // CLASSIC_MODEL_H
