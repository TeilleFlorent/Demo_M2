#ifndef SHADER_HPP
#define SHADER_HPP
#include "shader.hpp"
#endif

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"
#include "glm/ext.hpp"

#include <algorithm>
using namespace std;

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cmath>


#define myPI 3.141593
#define myPI_2 1.570796






struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 BiTangent;
};


struct Texture {
    GLuint id;
    string type;
    aiString path;
};


// MESH CLASSE
class Mesh {
public:

    //  Mesh Data  
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;
    float shininess;
    glm::vec2 _max_tex_coord;
    glm::vec2 _min_tex_coord;

    Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures, float shini_mesh, glm::vec2 max_tex_coord, glm::vec2 min_tex_coord);

    void Draw(Shader shader, int id); 
    

private:
  
    GLuint VAO, VBO, EBO;

    void setupMesh();
    
};

//////////////////////////////////

// MODEL CLASSE
class Model 
{
public:

    Assimp::Importer importer;
    const aiScene* scene;
    int NumVertices;

    Model();

    void Draw(Shader shader, glm::mat4 modelview2,bool test);   

    void Load_Model(string path, int id);

    void Print_info_model();

    vector<Mesh> meshes;
    string directory;
    vector<Texture> textures_loaded;    // variable bricolage optimisation (ne charge pas deux foix les meme texture)
    int model_id;
    
    void loadModel(string path);

    void processNode(aiNode* node, const aiScene* scene, int num_mesh);

    Mesh processMesh(aiMesh* mesh, const aiScene* scene, int num_mesh);

    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName, int num_mesh);
    
    GLint TextureFromFile(const char* path, string directory);
};    