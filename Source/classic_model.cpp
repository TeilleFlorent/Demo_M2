#include "classic_model.hpp"
#include "toolbox.hpp"


//******************************************************************************
//**********  Class Mesh  ******************************************************
//******************************************************************************

Mesh::Mesh( vector< Vertex > 			 iVertices,
            vector< unsigned int > iIndices,
            vector< Texture > 		 iTextures,
            glm::mat4       		 	 iLocalTransform )
{
  this->_vertices        = iVertices;
  this->_indices         = iIndices;
  this->_textures        = iTextures;
  this->_local_transform = iLocalTransform;
  this->SetupMesh();
}

void Mesh::Draw( Shader      iShader,
                 int         iModelID,
                 int         iMeshNumber,
                 glm::mat4   iModelMatrix ) 
{

  // Mesh corresponding texture binding
  // ----------------------------------
  for( unsigned int i = 0; i < this->_textures.size(); i++ )
  {
    int n; 
    string name = this->_textures[ i ]._type;

    // generate texture string uniform
    if( name == "uTextureDiffuse" )
    {
    	n = 0;
    }
    if( name == "uTextureNormal" )
    {
    	n = 1;
    }
    if( name == "uTextureHeight" )
    {
    	n = 2;
    }
    if( name == "uTextureAO" )
    {
    	n = 3;
    }
    if( name == "uTextureRoughness" )
    {
    	n = 4;
    }
    if( name == "uTextureMetalness" )
    {
    	n = 5;
    }
    if( name == "uTextureOpacity" )
    {
    	n = 6;
    }
    
    glActiveTexture( GL_TEXTURE0 + n );
    glBindTexture( GL_TEXTURE_2D, this->_textures[ i ]._id );
  }


  // Mesh Drawing
  // ------------
	glBindVertexArray( this->_VAO );
	
	glm::mat4 model_matrix;
	model_matrix = iModelMatrix * _local_transform;
	glUniformMatrix4fv( glGetUniformLocation( iShader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );

	if( iModelID == 2 )
  {
		// Transparent double sided mesh
		glCullFace( GL_FRONT );
		glDrawElements( GL_TRIANGLES, this->_indices.size(), GL_UNSIGNED_INT, 0 );
		glCullFace( GL_BACK );
		glDrawElements( GL_TRIANGLES, this->_indices.size(), GL_UNSIGNED_INT, 0 );
  }
  else
  {
		glDrawElements( GL_TRIANGLES, this->_indices.size(), GL_UNSIGNED_INT, 0 );
	}
	
	glBindVertexArray( 0 );

  // Unbind all used textures
  for( unsigned int i = 0; i < this->_textures.size(); i++ )
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
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, this->_indices.size() * sizeof( unsigned int ), &this->_indices[ 0 ], GL_STATIC_DRAW );

  // Vertex Positions
  glEnableVertexAttribArray( 0 );   
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( GLvoid* )0 );
  
  // Vertex Normals
  glEnableVertexAttribArray( 1 );   
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( GLvoid* )offsetof( Vertex, _normal ) );
  
  // Vertex UVs
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

Model::Model( string iPath, 
              int iID,
              string iName )
{
  _model_id = iID;
  _model_name = iName;
  LoadModel( iPath );
}

void Model::Draw( Shader      iShader,
									glm::mat4   iModelMatrix )
{
  for( unsigned int i = 0; i < this->_meshes.size(); i++ )
  {
    this->_meshes[ i ].Draw( iShader,
    											   this->_model_id,
    											   i,
    											   iModelMatrix );
  }
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

  // Get and print all supported file format 
  // ---------------------------------------
  
  //aiString all_supported_format;   
  //_importer.GetExtensionList( all_supported_format );
  //std::cout << all_supported_format.C_Str() << std::endl;   
  

  // Load model scene and process all nodes hierarchy
  // ------------------------------------------------
  _scene = _importer.ReadFile( iPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace );

  if( !_scene || _scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !_scene->mRootNode )
  {
    cout << "\n\nASSIMP ERROR => LoadModel() function\n\n" << _importer.GetErrorString() << endl;
    return;
  }

  this->_directory = iPath.substr( 0, iPath.find_last_of( '/' ) );
  this->ProcessNode( _scene->mRootNode, 0 );


  // Erase unwanted mesh
  // -------------------
  if( _model_id == 2 )
  {	
  	// delete [ 1 ]
  	_meshes.erase( _meshes.begin() + 1 );
  }

  if( _model_id == 3 )
  {	
  	// delete [ 1 ]
  	_meshes.erase( _meshes.begin() + 1 );
  }
}

void Model::ProcessNode( aiNode * iNode,
                         int iMeshNum )
{
  for( unsigned int i = 0; i < iNode->mNumMeshes; i++ )
  {
    aiMesh * mesh = _scene->mMeshes[ iNode->mMeshes[ i ] ]; 
    this->_meshes.push_back( this->ProcessMesh( mesh,
    																				    iMeshNum,
    																				    iNode->mTransformation ) );  
  }
   
  for( unsigned int i = 0; i < iNode->mNumChildren; i++ )
  {
    this->ProcessNode( iNode->mChildren[ i ], i );
  }
}

Mesh Model::ProcessMesh( aiMesh * 	 iMesh, 
                         int 				 iMeshNum,
                         aiMatrix4x4 iLocalTransform )
{
  vector< Vertex > vertices;
  vector< unsigned int > indices;
  vector< Texture > textures;
  bool UV_warning = false;


  // Load Mesh vertices data
  // -----------------------
  for( unsigned int i = 0; i < iMesh->mNumVertices; i++ )
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

    if( _model_id != 1 )
    { 
      // Assimp tangent
      vector.x = iMesh->mTangents[ i ].x;
      vector.y = iMesh->mTangents[ i ].y;
      vector.z = iMesh->mTangents[ i ].z;
      vertex._tangent = vector; 

      // Assimp bi tangent
      vector.x = iMesh->mBitangents[ i ].x;
      vector.y = iMesh->mBitangents[ i ].y;
      vector.z = iMesh->mBitangents[ i ].z;
      vertex._bi_tangent = vector;
    } 

    // Add this vertex to the mesh
    vertices.push_back( vertex );
  }

  if( UV_warning )
  {
    std::cout << std::endl << "WARNING : Model \"" << _model_name << "\", Mesh " << iMeshNum << " does not have UVs" << std::endl
                           << "--------------------------------------------------------";
  }


  // Manually calculate vertex tangent & bi tangent
  // ----------------------------------------------
  /*for( unsigned int i = 0; i < vertices.size(); i += 3 )
  { 
    if( _model_id == 1 )
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
  }*/


  // Get mesh indices
  // ----------------
  for( unsigned int i = 0; i < iMesh->mNumFaces; i++ )
  {
    aiFace face = iMesh->mFaces[ i ];
    for( unsigned int j = 0; j < face.mNumIndices; j++ )
    {
      indices.push_back( face.mIndices[ j ] );
    }
  }


  // Load model textures
  // -------------------
  vector< Texture > model_textures = this->LoadModelTextures( iMeshNum );
  textures.insert( textures.end(), model_textures.begin(), model_textures.end() );
	

	// Get mesh local transform matrix
  // -------------------------------
  glm::mat4 local_transform = _toolbox->AssimpMatrixToGlmMatrix( &iLocalTransform );
  //std::cout << "\nModel Name = " << _model_name << std::endl
  //					<< "Local transform mesh : " << iMeshNum << std::endl;
  //_toolbox->PrintMatrix( &local_transform );

  return Mesh( vertices,
  					   indices,
  					   textures,
  					   local_transform );
}

Texture Model::LoadTexture( string iTextureType,
                            string iTextureName,
                            int    iInternalFormat,
                            int    iFormat )
{
  Texture result_texture;

  result_texture._path = this->_directory + '/' + iTextureName;
  result_texture._id   = TextureFromFile( result_texture._path,
  																				iInternalFormat,
  																				iFormat ); 
  result_texture._type = iTextureType;

  this->_textures_loaded.push_back( result_texture );  

  return result_texture;
}

vector< Texture > Model::LoadModelTextures( int iMeshNum )
{
  vector< Texture > textures;

  
  // Load table1 textures 
  // --------------------
  if( this->_model_id == 0 )
  {
    // albedo
    textures.push_back( LoadTexture( "uTextureDiffuse",
                                     "albedo.png",
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     "normal.png",
                                     GL_RGB,
                                     GL_RGB ) );

    // height
    textures.push_back( LoadTexture( "uTextureHeight",
                                     "height.png",
                                     GL_R8,
                                     GL_RED ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     "AO.png",
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     "roughness.png",
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     "metalness.png",
                                     GL_R8,
                                     GL_RED ) );
  }


  // Load table1 textures 
  // --------------------
  if( this->_model_id == 2 )
  {
    // albedo
    textures.push_back( LoadTexture( "uTextureDiffuse",
                                     "albedo.png",
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     "normal.png",
                                     GL_RGB,
                                     GL_RGB ) );
    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     "AO.png",
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     "roughness.png",
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     "metalness.png",
                                     GL_R8,
                                     GL_RED ) );

    // opacity
    textures.push_back( LoadTexture( "uTextureOpacity",
                                     "opacity.png",
                                     GL_R8,
                                     GL_RED ) );
  }

  return textures;
}

unsigned int Model::TextureFromFile( string iTexturePath,
																		 int    iInternalFormat,
                            				 int    iFormat )
{

  // Check loaded texture vector to not load two times the same texture file
  // -----------------------------------------------------------------------
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

  unsigned int textureID;
  glGenTextures( 1, &textureID );
  t = IMG_Load( iTexturePath.c_str() );
  //std::cout << "path loaded =" << iTexturePath << std::endl;
	  
  if( !t )
  {
  	printf( "Loading image fail => image null\n" );
  }
  else
  {
    //std::cout << "Texture : " << iTexturePath << " => Loaded" << std::endl;
  }
  
  glBindTexture( GL_TEXTURE_2D, textureID );
  
  glTexImage2D( GL_TEXTURE_2D, 0, iInternalFormat, t->w, t->h, 0, iFormat, GL_UNSIGNED_BYTE, t->pixels );
 
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
