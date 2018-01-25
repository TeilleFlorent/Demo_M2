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
          glm::mat4         iLocalTransform );

    void Draw( Shader      iShader,
               int         iModelID,
               int         iMeshNumber,
               glm::mat4   iModelMatrix ); 

    
    // Class data
    // ---------  
    vector< Vertex >  _vertices;
    vector< GLuint >  _indices;
    vector< Texture > _textures;
    glm::mat4         _local_transform;


  private:
  
    unsigned int _VAO, _VBO, _EBO;

    void SetupMesh();
    
};


//******************************************************************************
//**********  Class Model  *****************************************************
//******************************************************************************

class Toolbox;

class Model 
{

  public:


    // Class functions
    // ---------------
    Model( string      iPath,
           int         iID,
           string      iName );

    void Draw( Shader      iShader,
               glm::mat4   iModelMatrix );   

    void Print_info_model();

    void LoadModel( string iPath );

    void ProcessNode( aiNode * iNode,
                      int      iMeshNum );

    Mesh ProcessMesh( aiMesh *    iMesh,
                      int         iMeshNum,
                      aiMatrix4x4 iLocalTransform );

    vector< Texture > LoadModelTextures( int iMeshNum );

    Texture LoadTexture( string iTextureType,
                         string iTextureName );
    
    unsigned int TextureFromFile( string iTexturePath );

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
    const aiScene *   _scene;


  private:

    // Pointer on the window program toolbox
    static Toolbox * _toolbox;

};    

#endif  // CLASSIC_MODEL_H
