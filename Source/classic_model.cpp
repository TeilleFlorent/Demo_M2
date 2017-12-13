#include "classic_model.hpp"
#include "toolbox.hpp"


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

Toolbox * Model::_toolbox; 

Model::Model()
{
}

void Model::Draw( Shader iShader )
{
  for( GLuint i = 0; i < this->_meshes.size(); i++ )
  {
    this->_meshes[ i ].Draw( iShader, this->_model_id );
  }
}
    
void Model::Load_Model( string iPath, 
                        int iID,
                        string iName )
{
  _model_id = iID;
  _model_name = iName;
  LoadModel( iPath );
}

void Model::Print_info_model()
{
  float res = 0;
  cout << "\n\nClassic model: " << "\"" << _model_name << "\"" << std::endl
       <<     "-------------- " << std::endl
       <<     "Mesh count : "   << _meshes.size() << endl;

  for( unsigned int i = 0; i < _meshes.size(); i++ )
  {
    cout << "Mesh " << i << " -> vertice count : " << _meshes[ i ]._vertices.size() << endl;
    res += _meshes[ i ]._vertices.size();
  }

  _vertice_count = res;
  cout << "Model total vertice count : " << res << "\n" << endl;
}

void Model::LoadModel( string iPath )
{
  // print tous les format supporté
  aiString all_supported_format;   
  _importer.GetExtensionList( all_supported_format );
  //std::cout << all_supported_format.C_Str() << std::endl;   
  
  _scene = _importer.ReadFile( iPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace );

  if( !_scene || _scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !_scene->mRootNode )
  {
    cout << "\n\nASSIMP ERROR => LoadModel() function\n\n" << _importer.GetErrorString() << endl;
    return;
  }

  this->_directory = iPath.substr( 0, iPath.find_last_of( '/' ) );
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
  bool UV_warning = false;


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
      UV_warning = true;
    }

    // Assimp tangent
    /*vector.x = iMesh->mTangents[ i ].x;
    vector.y = iMesh->mTangents[ i ].y;
    vector.z = iMesh->mTangents[ i ].z;
    vertex.Tangent = vector; 

    // Assimp bi tangent
    vector.x = iMesh->mBitangents[ i ].x;
    vector.y = iMesh->mBitangents[ i ].y;
    vector.z = iMesh->mBitangents[ i ].z;
    vertex.BiTangent = vector;*/ 

    // Add this vertex to the mesh
    vertices.push_back( vertex );
  }

  if( UV_warning )
  {
    std::cout << std::endl << "WARNING : Model \"" << _model_name << "\", Mesh " << iMeshNum << " does not have UVs" << std::endl
                           << "--------------------------------------------------------" << std::endl;
  }


  // Manually calculate vertex tangent & bi tangent
  // ----------------------------------------------
  for( GLuint i = 0; i < vertices.size(); i += 3 )
  { 
    if( _model_id == 2 )
    {
      break;
    }

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
  vector< Texture > model_textures = this->LoadModelTextures( iMeshNum );
  textures.insert( textures.end(), model_textures.begin(), model_textures.end() );
      
  return Mesh( vertices, indices, textures );
}

vector< Texture > Model::LoadModelTextures( int iMeshNum )
{
  vector< Texture > textures;
  Texture texture;
  string texture_name;
  string texture_path;
  string texture_type;

  
  // Load table1 textures 
  // --------------------
  if( this->_model_id == 0 )
  {
    // albedo
    texture_type  = "texture_diffuse"; 
    texture_name  = "albedo.png";            
    texture_path  = this->_directory + '/' + texture_name;
    texture._id   = TextureFromFile( texture_path );
    texture._path = texture_path;
    texture._type = texture_type;
    textures.push_back( texture );
    this->_textures_loaded.push_back( texture );  

    // normal
    texture_type  = "texture_normal"; 
    texture_name  = "normal.png";            
    texture_path  = this->_directory + '/' + texture_name;
    texture._id   = TextureFromFile( texture_path );
    texture._path = texture_path;
    texture._type = texture_type;
    textures.push_back( texture );
    this->_textures_loaded.push_back( texture ); 

    // height
    texture_type  = "texture_height"; 
    texture_name  = "height.png";            
    texture_path  = this->_directory + '/' + texture_name;
    texture._id   = TextureFromFile( texture_path );
    texture._path = texture_path;
    texture._type = texture_type;
    textures.push_back( texture );
    this->_textures_loaded.push_back( texture );

    // AO
    texture_type  = "texture_AO"; 
    texture_name  = "AO.png";            
    texture_path  = this->_directory + '/' + texture_name;
    texture._id   = TextureFromFile( texture_path );
    texture._path = texture_path;
    texture._type = texture_type;
    textures.push_back( texture );
    this->_textures_loaded.push_back( texture );

    // roughness 
    texture_type  = "texture_roughness"; 
    texture_name  = "roughness.png";            
    texture_path  = this->_directory + '/' + texture_name;
    texture._id   = TextureFromFile( texture_path );
    texture._path = texture_path;
    texture._type = texture_type;
    textures.push_back( texture );
    this->_textures_loaded.push_back( texture );

    // metalness
    texture_type  = "texture_metalness"; 
    texture_name  = "metalness.png";            
    texture_path  = this->_directory + '/' + texture_name;
    texture._id   = TextureFromFile( texture_path );
    texture._path = texture_path;
    texture._type = texture_type;
    textures.push_back( texture );
    this->_textures_loaded.push_back( texture );

    return textures;
  }

}

GLuint Model::TextureFromFile( string iTexturePath )
{

  // Check loaded texture vector and for not load two times the same texture file
  // ----------------------------------------------------------------------------
  for( unsigned int i = 0; i < _textures_loaded.size(); i++ )
  {
    if( iTexturePath.compare( _textures_loaded[ i ]._path ) == 0 )
    {
      return _textures_loaded[ i ]._id;
    }  
  }


  // Load new texture file
  // ---------------------
  float aniso;
  glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso ); // get la valeur pour l'aniso

  SDL_Surface * t = NULL;

  GLuint textureID;
  glGenTextures( 1, &textureID );
  t = IMG_Load( iTexturePath.c_str() );

  if( !t )
  {
    printf( "Loading image fail => image null\n" );
  }
  else
  {
    //std::cout << "Texture : " << iTexturePath << " => Loaded" << std::endl;
  }
  
  glBindTexture( GL_TEXTURE_2D, textureID );
  if( _toolbox->IsTextureRGBA( t ) )
  {
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels );
  }
  else
  {
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels );
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

void Model::SetToolbox( Toolbox * iToolbox )
{
  _toolbox = iToolbox;
}

Toolbox * Model::GetToolbox()
{
  return _toolbox;
}
