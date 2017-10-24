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
  
    GLuint _id;
    string _type;
    string _path;
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
          vector< Texture > iTextures );

    void Draw( Shader iShader,
               int    iID ); 

    
    // Class data
    // ---------  
    vector< Vertex >  _vertices;
    vector< GLuint >  _indices;
    vector< Texture > _textures;


  private:
  
    GLuint _VAO, _VBO, _EBO;

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
    Model();

    void Draw( Shader iShader );   

    void Load_Model( string iPath,
                     int    iID,
                     string iName );

    void Print_info_model();

    void LoadModel( string iPath );

    void ProcessNode( aiNode * iNode,
                      int      iMeshNum );

    Mesh ProcessMesh( aiMesh* iMesh,
                      int     iMeshNum );

    vector< Texture > LoadModelTextures( int iMeshNum );
    
    GLuint TextureFromFile( string iTexturePath );

    static void SetToolbox( Toolbox * iToolbox );

    static Toolbox * GetToolbox();


    // Class data
    // ----------
    vector< Mesh >    _meshes;
    string            _directory;
    vector< Texture > _textures_loaded;
    int               _model_id;
    Assimp::Importer  _importer;
    const aiScene *   _scene;
    int               _vertice_count;
    string            _model_name; 


  private:

    // Pointer on the window program toolbox
    static Toolbox * _toolbox;

};    

#endif  // CLASSIC_MODEL_H
