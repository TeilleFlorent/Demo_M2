#include "classic_model.hpp"


//******************************************************************************
//**********  Class Mesh  ******************************************************
//******************************************************************************

Mesh::Mesh( vector< Vertex > iVertices,
            vector< GLuint > iIndices,
            vector< Texture > iTextures )
{
  this->_vertices = iVertices;
  this->_indices = iIndices;
  this->_textures = iTextures;
  this->SetupMesh();
}

void Mesh::Draw( Shader iShader,
                 int iID ) 
{
  // var pour bind la bonne tex
  GLuint diffuseNr = 1;
  GLuint specularNr = 1;
  GLuint normalNr = 1;
  GLuint heightNr = 1;
  GLuint AONr = 1;
  GLuint roughnessNr = 1;
  GLuint metalnessNr = 1;

  // Mesh corresponding texture binding
  // ----------------------------------
  for( GLuint i = 0; i < this->_textures.size(); i++ )
  {
    glActiveTexture( GL_TEXTURE0 + i ); // active la texture qu'il faut

    stringstream ss;
    string number;
    string name = this->_textures[ i ]._type;

    //std::cout << "tex = " << name << ", i = " << i << std::endl;

    // generate texture string uniform
    if( name == "uTextureDiffuse" )
        ss << diffuseNr++; 
    if( name == "uTextureNormal" )
        ss << normalNr++; 
    if( name == "uTextureHeight" )
        ss << heightNr++; 
    if( name == "uTextureAO" )
        ss << AONr++; 
    if( name == "uTextureRoughness" )
        ss << roughnessNr++; 
    if( name == "uTextureMetalness" )
        ss << metalnessNr++; 
    if( name == "uTextureSpecular" )
        ss << specularNr++; 
               
    number = ss.str(); 

    //std::cout << "uniform sampler name = " << (name + number) << std::endl;

    glUniform1i( glGetUniformLocation( iShader._program, ( name + number ).c_str() ), i );
    glBindTexture( GL_TEXTURE_2D, this->_textures[ i ]._id ); // bind la tex qui correspond au string generer et envoyer au juste avant
  }

  // Mesh Drawing
  // ------------
  glBindVertexArray( this->_VAO );
  glDrawElements( GL_TRIANGLES, this->_indices.size(), GL_UNSIGNED_INT, 0 );
  //glDrawArrays(GL_TRIANGLES, 0, this->indices.size());
  glBindVertexArray( 0 );


  // Unbind all used textures
  for( GLuint i = 0; i < this->_textures.size(); i++ )
  {
    glActiveTexture( GL_TEXTURE0 + i );
    glBindTexture( GL_TEXTURE_2D, 0 );
  }
}

void Mesh::SetupMesh()
{
  glGenVertexArrays( 1, &this->_VAO );
  glGenBuffers( 1, &this->_VBO );
  glGenBuffers( 1, &this->_EBO );

  glBindVertexArray( this->_VAO );
 
  glBindBuffer( GL_ARRAY_BUFFER, this->_VBO );
  glBufferData( GL_ARRAY_BUFFER, this->_vertices.size() * sizeof(Vertex), &this->_vertices[ 0 ], GL_STATIC_DRAW );  

  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, this->_EBO );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, this->_indices.size() * sizeof( GLuint ), &this->_indices[ 0 ], GL_STATIC_DRAW );

  // Vertex Positions
  glEnableVertexAttribArray( 0 );   
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( GLvoid* )0 );
  
  // Vertex Normals
  glEnableVertexAttribArray( 1 );   
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( GLvoid* )offsetof( Vertex, _normal ) );
  
  // Vertex Texture Coords
  glEnableVertexAttribArray( 2 );   
  glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( GLvoid* )offsetof( Vertex, _uv ) );
  
  // Vertex Tangent
  glEnableVertexAttribArray( 3 );   
  glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( GLvoid* )offsetof( Vertex, _tangent ) );

  // Vertex Bi Tangent
  glEnableVertexAttribArray( 4 );   
  glVertexAttribPointer( 4, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( GLvoid* )offsetof( Vertex, _bi_tangent ) );

  glBindVertexArray( 0 );
}


//******************************************************************************
//**********  Class Model  *****************************************************
//******************************************************************************

Model::Model()
{
}

// draw tout les meshes du model
void Model::Draw( Shader iShader )
{
  for( GLuint i = 0; i < this->_meshes.size(); i++ )
  {
    this->_meshes[ i ].Draw( iShader, this->_model_id );
  }
}
    
void Model::Load_Model( string iPath, 
                        int iID )
{

  this->_model_id = iID;
  this->LoadModel( iPath );
}

void Model::Print_info_model()
{
  float res = 0;
  cout << "\nCLASSIC MODEL:\n" << "nbMeshes = " << _meshes.size() << endl;

  for( unsigned int i = 0; i < _meshes.size(); i++ )
  {
    cout << "mesh " << i << ", nbVertices = " << _meshes[ i ]._vertices.size() << endl;
    res += _meshes[ i ]._vertices.size();
  }

  _vertice_count = res;
  cout << "nb_vertices_total = " << res << "\n" << endl;
}

void Model::LoadModel( string iPath )
{
  // print tous les format supporté
  aiString all_supported_format;   
  _importer.GetExtensionList( all_supported_format );
  //cout << bla.C_Str() << endl;   
  
  _scene = _importer.ReadFile( iPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace );

  if( !_scene || _scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !_scene->mRootNode )
  {
      cout << "\n\nERROR ASSIMP => LoadModel() function\n\n" << _importer.GetErrorString() << endl;
      return;
  }

  this->_directory = iPath.substr( 0, iPath.find_last_of('/') );
  this->ProcessNode( _scene->mRootNode, 0 );
}

void Model::ProcessNode( aiNode * iNode,
                         int iMeshNum )
{
  for( GLuint i = 0; i < iNode->mNumMeshes; i++ )
  {
    aiMesh * mesh = _scene->mMeshes[ iNode->mMeshes[ i ] ]; 
    this->_meshes.push_back( this->ProcessMesh( mesh, iMeshNum) );  
  }
   
  for( GLuint i = 0; i < iNode->mNumChildren; i++ )
  {
    this->ProcessNode( iNode->mChildren[ i ], i );
  }
}

Mesh Model::ProcessMesh( aiMesh * iMesh, 
                         int iMeshNum )
{
  vector< Vertex > vertices;
  vector< GLuint > indices;
  vector< Texture > textures;


  // Load Mesh vertices data
  // -----------------------
  for( GLuint i = 0; i < iMesh->mNumVertices; i++ )
  {
    Vertex vertex;
    glm::vec3 vector; 

    // Position
    vector.x = iMesh->mVertices[ i ].x;
    vector.y = iMesh->mVertices[ i ].y;
    vector.z = iMesh->mVertices[ i ].z;
    vertex._position = vector;

    // Normal
    vector.x = iMesh->mNormals[ i ].x;
    vector.y = iMesh->mNormals[ i ].y;
    vector.z = iMesh->mNormals[ i ].z;
    vertex._normal = vector;

    // Texture Coord
    if( iMesh->mTextureCoords[ 0 ] ) // verifie si il y a des tex coord
    {
      glm::vec2 vec;
      vec.x = iMesh->mTextureCoords[ 0 ][ i ].x; 
      vec.y = iMesh->mTextureCoords[ 0 ][ i ].y;
      vertex._uv = vec;
    }
    else
    {
      vertex._uv = glm::vec2( 0.0f, 0.0f );
      printf( "\n\nThere is no UV\n\n" );
    }

    // Assimp tangent
    vector.x = iMesh->mTangents[ i ].x;
    vector.y = iMesh->mTangents[ i ].y;
    vector.z = iMesh->mTangents[ i ].z;
    //vertex.Tangent = vector; 

    // Assimp bi tangent
    vector.x = iMesh->mBitangents[ i ].x;
    vector.y = iMesh->mBitangents[ i ].y;
    vector.z = iMesh->mBitangents[ i ].z;
    //vertex.BiTangent = vector; 

    // Add this vertex to the mesh
    vertices.push_back( vertex );
  }


  // Manually calculate vertex tangent & bi tangent
  // ----------------------------------------------
  for( GLuint i = 0; i < vertices.size(); i += 3 )
  {
    // Raccourcis pour les sommets
    glm::vec3 v0,v1,v2;
    v0 = vertices[ i ]._position;
    v1 = vertices[ i + 1 ]._position;
    v2 = vertices[ i + 2 ]._position; 

    // Raccourcis pour les UV
    glm::vec2 uv0,uv1,uv2;
    uv0 = vertices[ i ]._uv;
    uv1 = vertices[ i + 1 ]._uv;
    uv2 = vertices[ i + 2 ]._uv;

    // Côtés du triangle : delta des positions
    glm::vec3 deltaPos1 = v1 - v0; 
    glm::vec3 deltaPos2 = v2 - v0; 

    // delta UV
    glm::vec2 deltaUV1 = uv1 - uv0; 
    glm::vec2 deltaUV2 = uv2 - uv0;

    float r = 1.0f / ( deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x ); 
    glm::vec3 tangent = ( deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y ) * r; 
    glm::vec3 bitangent = ( deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x ) * r;

    vertices[ i ]._tangent = tangent; 
    vertices[ i + 1 ]._tangent = tangent;
    vertices[ i + 2 ]._tangent = tangent;

    vertices[ i ]._bi_tangent = bitangent; 
    vertices[ i + 1 ]._bi_tangent = bitangent;
    vertices[ i + 2 ]._bi_tangent = bitangent;

  }


  // Get mesh indices
  // ----------------
  for( GLuint i = 0; i < iMesh->mNumFaces; i++ )
  {
    aiFace face = iMesh->mFaces[ i ];
    for( GLuint j = 0; j < face.mNumIndices; j++ )
    {
      indices.push_back( face.mIndices[ j ] );
    }
  }


  // Load model textures
  // -------------------
  aiMaterial * material = _scene->mMaterials[ iMesh->mMaterialIndex ];

  vector< Texture > diffuseMaps = this->LoadMaterialTextures( material, aiTextureType_DIFFUSE, "texture_diffuse", iMeshNum );
  textures.insert( textures.end(), diffuseMaps.begin(), diffuseMaps.end() );

  /*if(model_id != 0){
  // Specular maps
  vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", num_mesh);
  textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  // Specular maps
  vector<Texture> normalMaps = this->loadMaterialTextures(material, aiTextureType_HEIGHT , "texture_normal", num_mesh);
  textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
  }*/
      
  return Mesh( vertices, indices, textures );
}

vector< Texture > Model::LoadMaterialTextures( aiMaterial * iMaterial,
                                               aiTextureType iType,
                                               string iTypeName,
                                               int iMeshNum )
{

  vector< Texture > textures;

  // Load table1 textures 
  // --------------------
  if( this->_model_id == 0 )
  {
    GLboolean skip = false;
    string temp1;
    aiString str;

    if( !skip )
    {
      // albedo
      temp1 = "albedo.png";            
      str.Set( temp1 );
      Texture texture;
      texture._id = TextureFromFile( str.C_Str() , this->_directory );
      texture._path = str;
      temp1 = "texture_diffuse";
      texture._type = temp1;
      textures.push_back( texture );
      this->_textures_loaded.push_back( texture );  

      // normal
      temp1 = "normal.png";            
      str.Set( temp1 );
      texture._id = TextureFromFile( str.C_Str() , this->_directory );
      texture._path = str;
      temp1 = "texture_normal";
      texture._type = temp1;
      textures.push_back( texture );
      this->_textures_loaded.push_back( texture ); 

      // height
      temp1 = "height.png";            
      str.Set( temp1 );
      texture._id = TextureFromFile( str.C_Str() , this->_directory );
      texture._path = str;
      temp1 = "texture_height";
      texture._type = temp1;
      textures.push_back( texture );
      this->_textures_loaded.push_back( texture );

      // AO
      temp1 = "AO.png";            
      str.Set( temp1 );
      texture._id = TextureFromFile( str.C_Str() , this->_directory );
      texture._path = str;
      temp1 = "texture_AO";
      texture._type = temp1;
      textures.push_back( texture );
      this->_textures_loaded.push_back( texture );

      // roughness
      temp1 = "roughness.png";            
      str.Set( temp1 );
      texture._id = TextureFromFile( str.C_Str() , this->_directory );
      texture._path = str;
      temp1 = "texture_roughness";
      texture._type = temp1;
      textures.push_back( texture );
      this->_textures_loaded.push_back( texture );  

      // metalness
      temp1 = "metalness.png";            
      str.Set( temp1 );
      texture._id = TextureFromFile( str.C_Str() , this->_directory );
      texture._path = str;
      temp1 = "texture_metalness";
      texture._type = temp1;
      textures.push_back( texture );
      this->_textures_loaded.push_back( texture );  
    }

    return textures;
  }


  // Optimisation texture loading system
  // -----------------------------------
  for( GLuint i = 0; i < iMaterial->GetTextureCount( iType ); i++ )
  {
    aiString str;
    iMaterial->GetTexture( iType, i, &str );
    
    string test_path = str.data;
    GLboolean skip = false;
 
    for( GLuint j = 0; j < _textures_loaded.size(); j++ )
    {
      if( _textures_loaded[ j ]._path == str )
      {
        textures.push_back( _textures_loaded[ j ] );
        skip = true; 
        break;
      }
    }
  
    if( !skip )
    {   
      Texture texture;
      texture._id = TextureFromFile( str.C_Str(), this->_directory );
      texture._type = iTypeName;
      texture._path = str;
      textures.push_back( texture );
      this->_textures_loaded.push_back( texture );  
    }
  }

  return textures;
}

GLint Model::TextureFromFile( const char * iPath,
                              string iDirectory )
{

  float aniso;
  glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso ); // get la valeur pour l'aniso

  SDL_Surface * t = NULL;

  string filename = string( iPath );
  filename = iDirectory + '/' + filename;
  GLuint textureID;
  glGenTextures( 1, &textureID );
  t = IMG_Load( filename.c_str() );

  /*
  std::cout << "test1 = " << path << std::endl;
  std::cout << "test2 = " << directory << std::endl;     
  std::cout << "test3 = " << filename << std::endl;  
  if( !t )
  {
    printf("image null\n");
  }
  */
 
  glBindTexture( GL_TEXTURE_2D, textureID );

  if( t->format->format == SDL_PIXELFORMAT_RGB332
   || t->format->format == SDL_PIXELFORMAT_RGB444
   || t->format->format == SDL_PIXELFORMAT_RGB555
   || t->format->format == SDL_PIXELFORMAT_RGB565
   || t->format->format == SDL_PIXELFORMAT_RGB24
   || t->format->format == SDL_PIXELFORMAT_RGB888
  //|| t->format->format == SDL_PIXELFORMAT_RGBX8888
   || t->format->format == SDL_PIXELFORMAT_RGB565
   || t->format->format == SDL_PIXELFORMAT_BGR555
   || t->format->format == SDL_PIXELFORMAT_BGR565
   || t->format->format == SDL_PIXELFORMAT_BGR24
   || t->format->format == SDL_PIXELFORMAT_BGR888 )
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
  }
  else
  {
    if( t->format->format == SDL_PIXELFORMAT_RGBA4444
     || t->format->format == SDL_PIXELFORMAT_RGBA5551
     || t->format->format == SDL_PIXELFORMAT_ARGB4444
     || t->format->format == SDL_PIXELFORMAT_ABGR4444
     || t->format->format == SDL_PIXELFORMAT_BGRA4444
     || t->format->format == SDL_PIXELFORMAT_ABGR1555
     || t->format->format == SDL_PIXELFORMAT_BGRA5551
     || t->format->format == SDL_PIXELFORMAT_ARGB8888
     || t->format->format == SDL_PIXELFORMAT_ABGR8888
     || t->format->format == SDL_PIXELFORMAT_BGRA8888
    //|| t->format->format == SDL_PIXELFORMAT_RGBX8888
     || t->format->format == SDL_PIXELFORMAT_RGBA8888 )
    {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels );
    }
    else
    { 
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels );
    }
  }

  glGenerateMipmap( GL_TEXTURE_2D );    
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso ); // anisotropie

  glBindTexture( GL_TEXTURE_2D, 0 );
  SDL_FreeSurface( t );
  return textureID;
}
