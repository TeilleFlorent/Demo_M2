#include "classic_model.hpp"
#include "toolbox.hpp"


//******************************************************************************
//**********  Class Mesh  ******************************************************
//******************************************************************************

Mesh::Mesh( vector< Vertex > 			 iVertices,
            vector< unsigned int > iIndices,
            vector< Texture > 		 iTextures,
            glm::mat4       		 	 iLocalTransform,
            aiString               iMeshName,
            bool 									 iOpacityMap )
{
  this->_vertices        = iVertices;
  this->_indices         = iIndices;
  this->_textures        = iTextures;
  this->_local_transform = iLocalTransform;
  this->_name            = iMeshName;  
  this->_opacity_map     = iOpacityMap;
  this->SetupMesh();
}

void Mesh::Draw( Shader    iShader,
                 int       iModelID,
                 int       iMeshNumber,
                 glm::mat4 iModelMatrix,
                 bool      iNormalMap,
                 bool      iHeightMap,
                 float     iOpacityDiscard ) 
{
	glUniform1i( glGetUniformLocation( iShader._program, "uNormalMap" ), false );
	glUniform1i( glGetUniformLocation( iShader._program, "uOpacityMap" ), false );
  glUniform1f( glGetUniformLocation( iShader._program, "uOpacityDiscard" ), iOpacityDiscard );


  // Mesh corresponding texture binding
  // ----------------------------------
  for( unsigned int i = 0; i < this->_textures.size(); i++ )
  {
    int n; 
    string name = this->_textures[ i ]._type;

    // generate texture string uniform
    if( name == "uTextureAlbedo" )
    {
    	n = 0;
    }
    if( name == "uTextureNormal" )
    {
    	n = 1;
  		glUniform1i( glGetUniformLocation( iShader._program, "uNormalMap" ), iNormalMap );
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
    	glUniform1i( glGetUniformLocation( iShader._program, "uOpacityMap" ), true );
    }
    if( name == "uTextureEmissive" )
    {
      n = 11;
    }
    
    glActiveTexture( GL_TEXTURE0 + n );
    glBindTexture( GL_TEXTURE_2D, this->_textures[ i ]._id );
  }


  // Mesh Drawing
  // ------------
	glBindVertexArray( this->_VAO );
	
	// Perform mesh local transform
	glm::mat4 model_matrix;
	model_matrix = iModelMatrix * _local_transform;

  if( iModelID == 4 )
  {
    model_matrix = iModelMatrix;
  }

	glUniformMatrix4fv( glGetUniformLocation( iShader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );

	// Draw only transparent mesh part
	if( iOpacityDiscard == 2.0 )
  {
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    ( iHeightMap == true ) ? glDrawElements( GL_PATCHES, this->_indices.size(), GL_UNSIGNED_INT, 0 ) : glDrawElements( GL_TRIANGLES, this->_indices.size(), GL_UNSIGNED_INT, 0 );

		glDisable( GL_BLEND );
  }
  else
  {
    ( iHeightMap == true ) ? glDrawElements( GL_PATCHES, this->_indices.size(), GL_UNSIGNED_INT, 0 ) : glDrawElements( GL_TRIANGLES, this->_indices.size(), GL_UNSIGNED_INT, 0 );
  }
	
	glBindVertexArray( 0 );

  // Unbind all used textures
  for( unsigned int i = 0; i < this->_textures.size(); i++ )
  {
    glActiveTexture( GL_TEXTURE0 + i );
    glBindTexture( GL_TEXTURE_2D, 0 );
  }
}

void Mesh::DrawDepth( Shader    iShader,
                      glm::mat4 iModelMatrix )
{

  // Mesh Drawing
  // ------------
  glBindVertexArray( this->_VAO );
  
  // Perform mesh local transform
  glm::mat4 model_matrix;
  model_matrix = iModelMatrix * _local_transform;
  glUniformMatrix4fv( glGetUniformLocation( iShader._program, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( model_matrix ) );

  // Draw
  glDrawElements( GL_TRIANGLES, this->_indices.size(), GL_UNSIGNED_INT, 0 );
  
  glBindVertexArray( 0 );
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
Scene   * Model::_scene; 

Model::Model( string  iPath, 
              int     iID,
              string  iName,
              bool    iNormalMap,
              bool    iHeightMap )
{
  _model_id   = iID;
  _model_name = iName;
  _normal_map = iNormalMap;
  _height_map = iHeightMap;

  LoadModel( iPath );
}

void Model::Draw( Shader    iShader,
									glm::mat4 iModelMatrix )
{ 
  glm::mat4 * model_matrix;
  glm::mat4 rotation_matrix;
  glm::mat4 rotation_matrix1;
  glm::mat4 rotation_matrix2;
  glm::mat4 translation_matrix1;
  glm::mat4 translation_matrix2;

  // Set revolving door rotations
  if( _model_id == 3 )
  { 
    rotation_matrix  = iModelMatrix * _scene->_door_rotation_matrix;
    rotation_matrix1 = iModelMatrix * _scene->_door1_rotation_matrix;
    rotation_matrix2 = iModelMatrix * _scene->_door2_rotation_matrix;
  }

  // Set simple door translations
  if( _model_id == 4 )
  { 
    translation_matrix1 = iModelMatrix * _scene->_door_translation_matrix1;
    translation_matrix2 = iModelMatrix * _scene->_door_translation_matrix2;
  }

	// Draw non transparent model parts
  for( unsigned int i = 0; i < this->_meshes.size(); i++ )
  {  
    model_matrix = &iModelMatrix;

    // Perform revolving door rotations
    if( _model_id == 3 )
    {
      if( i > 14 && i < 18 )
      {
        model_matrix = &rotation_matrix;
      }

      if( i > 2 && i < 6 )
      {
        model_matrix = &rotation_matrix1;
      }

      if( i > 5 && i < 9 )
      {
        model_matrix = &rotation_matrix1;
      }

      if( i > 8 && i < 12 )
      {
        model_matrix = &rotation_matrix2;
      }

      if( i > 11 && i < 15 )
      {
        model_matrix = &rotation_matrix2;
      }
    }

    // Perform simple door translations
    if( _model_id == 4 )
    {
      if( i == 4 )
      {
        model_matrix = &translation_matrix1;
      }

      if( i == 5 )
      {
        model_matrix = &translation_matrix2;
      }
    }

    this->_meshes[ i ].Draw( iShader,
    											   this->_model_id,
    											   i,
    											   *model_matrix,
                             _normal_map,
                             _height_map,
    											   1.0 );
  }

	// Draw transparent model parts
  for( int i = this->_meshes.size() - 1; i >= 0.0; --i )
  {	
    model_matrix = &iModelMatrix;

    // Perform revolving door rotations
    if( _model_id == 3 )
    {
      if( i > 14 && i < 18 )
      {
        model_matrix = &rotation_matrix;
      }

      if( i > 2 && i < 6 )
      {
        model_matrix = &rotation_matrix1;
      }

      if( i > 5 && i < 9 )
      {
        model_matrix = &rotation_matrix1;
      }

      if( i > 8 && i < 12 )
      {
        model_matrix = &rotation_matrix2;
      }

      if( i > 11 && i < 15 )
      {
        model_matrix = &rotation_matrix2;
      }
    }
    
  	if( this->_meshes[ i ]._opacity_map )
  	{
	    this->_meshes[ i ].Draw( iShader,
	    											   this->_model_id,
	    											   i,
	    											   *model_matrix,
                               _normal_map,
                               _height_map,
	    											   2.0 );
	  }
  }
}

void Model::DrawDepth( Shader    iShader,
                       glm::mat4 iModelMatrix )
{
  
  glm::mat4 * model_matrix;
  glm::mat4 rotation_matrix;
  glm::mat4 rotation_matrix1;
  glm::mat4 rotation_matrix2;

  // Set revolving door rotation
  if( _model_id == 3 )
  { 
    rotation_matrix  = iModelMatrix * _scene->_door_rotation_matrix;
    rotation_matrix1 = iModelMatrix * _scene->_door1_rotation_matrix;
    rotation_matrix2 = iModelMatrix * _scene->_door2_rotation_matrix;
  }

  for( unsigned int i = 0; i < this->_meshes.size(); i++ )
  { 
    model_matrix = &iModelMatrix;

    // Perform revolving door rotation
    if( _model_id == 3 )
    {
      if( i > 14 && i < 18 )
      {
        model_matrix = &rotation_matrix;
      }

      if( i > 2 && i < 6 )
      {
        model_matrix = &rotation_matrix1;
      }

      if( i > 5 && i < 9 )
      {
        model_matrix = &rotation_matrix1;
      }

      if( i > 8 && i < 12 )
      {
        model_matrix = &rotation_matrix2;
      }

      if( i > 11 && i < 15 )
      {
        model_matrix = &rotation_matrix2;
      }
    }

    if( _model_id == 3 && this->_meshes[ i ]._opacity_map )
    {
      continue;
    }

    this->_meshes[ i ].DrawDepth( iShader,
                                  *model_matrix );
  }
}

void Model::PrintInfos()
{
  float res = 0;
  cout << "\n\nClassic model: " << "\"" << _model_name << "\"" << std::endl
       <<     "-------------- " << std::endl
       <<     "Mesh count : "   << _meshes.size() << endl;

  for( unsigned int i = 0; i < _meshes.size(); i++ )
  {
    cout << "Mesh " << i << " -> " << "\"" << _meshes[ i ]._name.C_Str() << "\"" << " : " << _meshes[ i ]._vertices.size() << " vertices" << endl;
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
  _assimp_scene = _importer.ReadFile( iPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace );

  if( !_assimp_scene || _assimp_scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !_assimp_scene->mRootNode )
  {
    cout << "\n\nASSIMP ERROR => LoadModel() function\n\n" << _importer.GetErrorString() << endl;
    return;
  }

  this->_directory = iPath.substr( 0, iPath.find_last_of( '/' ) );
  this->ProcessNode( _assimp_scene->mRootNode );


  // Erase unwanted mesh
  // -------------------
  if( _model_id == 2 )
  {	
  	// delete [ 1 ]
  	_meshes.erase( _meshes.begin() + 1 );
  }

  if( _model_id == 12 )
  {
    // delete [ 9 ]
    _meshes.erase( _meshes.begin() + 9 );
  }

  if( _model_id == 22 )
  {
    // delete [ 0 ]
    _meshes.erase( _meshes.begin() + 0 );
  }
}

void Model::ProcessNode( aiNode * iNode )
{ 
  for( unsigned int i = 0; i < iNode->mNumMeshes; i++ )
  {
    aiMesh * mesh = _assimp_scene->mMeshes[ iNode->mMeshes[ i ] ]; 
    this->_meshes.push_back( this->ProcessMesh( mesh,
    																				    iNode->mTransformation,
    																				    iNode->mName ) );  
  }
   
  for( unsigned int i = 0; i < iNode->mNumChildren; i++ )
  {
    this->ProcessNode( iNode->mChildren[ i ] );
  }
}

Mesh Model::ProcessMesh( aiMesh * 	 iMesh,
                         aiMatrix4x4 iLocalTransform,
                         aiString    iNodeName )
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
    std::cout << std::endl << "WARNING : Model \"" << _model_name << "\", Mesh : " << "\"" << iNodeName.C_Str() << "\"" << " does not have UVs" << std::endl
                           << "-----------------------------------------------------------------";
  }


  // Manually calculate vertex tangent & bi tangent
  // ----------------------------------------------
  if( false )
  {
    for( unsigned int i = 0; i < vertices.size(); i += 3 )
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
  }


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
  bool opacity_map = false;
  vector< Texture > model_textures = this->LoadMeshTextures( iNodeName,
  																											     &opacity_map,
                                                             iMesh->mMaterialIndex );
  textures.insert( textures.end(), model_textures.begin(), model_textures.end() );
	

	// Get mesh local transform matrix
  // -------------------------------
  glm::mat4 local_transform = _toolbox->AssimpMatrixToGlmMatrix( &iLocalTransform );
  //std::cout << "\nModel Name = " << _model_name << std::endl
  //					<< "Local transform mesh : " << "\"" << iNodeName << "\"" << std::endl;
  //_toolbox->PrintMatrix( &local_transform );

  return Mesh( vertices,
  					   indices,
  					   textures,
  					   local_transform,
  					   iMesh->mName,
  					   opacity_map );
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

vector< Texture > Model::LoadMeshTextures( aiString     iNodeName,
																					 bool *       oOpacityMap,
                                           unsigned int iMaterialIndex )
{
  vector< Texture > textures;


  // Load table1 textures 
  // --------------------
  if( this->_model_id == 0 )
  {
    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
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


  // Load ink bottle textures 
  // ------------------------
  if( this->_model_id == 2 )
  {
    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
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

    *oOpacityMap = true;
  }


  // Load revolving door textures 
  // ----------------------------
  if( this->_model_id == 3 )
  { 
    // normal
    std::string texture_name = ( iMaterialIndex == 0 ) ? std::string( "normal1.png" ) : std::string( "normal2.png" );
    if( iMaterialIndex != 2 )
    {
      textures.push_back( LoadTexture( "uTextureNormal",
                                       texture_name,
                                       GL_RGB,
                                       GL_RGB ) );
    }

    // albedo
    texture_name = ( iMaterialIndex == 0 ) ? std::string( "albedo1.png" ) : std::string( "albedo2.png" );
    if( iMaterialIndex == 2 )
    {
      texture_name = std::string( "albedo3.png" );      
    }
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     texture_name,
                                     GL_RGB,
                                     GL_RGB ) );

    // metalness & roughness
    texture_name = ( iMaterialIndex == 0 ) ? std::string( "MetaRough1.png" ) : std::string( "MetaRough2.png" );
    if( iMaterialIndex == 2 )
    {
      texture_name = std::string( "MetaRough3.png" );
    }

    textures.push_back( LoadTexture( "uTextureMetalness",
                                     texture_name,
                                     GL_RGBA,
                                     GL_RGBA ) );

    // opacity
    if( iMaterialIndex == 2 )
    {
      textures.push_back( LoadTexture( "uTextureOpacity",
                                       "opacity.png",
                                       GL_R8,
                                       GL_RED ) );

      *oOpacityMap = true;
    }
  }


  // Load simple door textures 
  // -------------------------
  if( this->_model_id == 4 )
  { 
    std::string texture_name( "" );
    
    if( iMaterialIndex == 3 )
    {
      texture_name += "door_";
    }

    if( iMaterialIndex == 0 )
    {
      texture_name += "bot_";
    }

    if( iMaterialIndex == 1 )
    {
      return textures;
    }

    if( iMaterialIndex == 2 )
    {
      texture_name += "frame_";
    }
    

    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );
    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );
  }


  // Load top light textures 
  // -----------------------
  if( this->_model_id == 5 )
  { 
    std::string texture_name( "" );
    
    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );
    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // emissive
    textures.push_back( LoadTexture( "uTextureEmissive",
                                     std::string( texture_name + "emissive.png" ),
                                     GL_RGB,
                                     GL_RGB ) );
  }


  // Load wall light textures 
  // ------------------------
  if( this->_model_id == 6 )
  { 
    std::string texture_name( "" );
    
    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );
    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // emissive
    textures.push_back( LoadTexture( "uTextureEmissive",
                                     std::string( texture_name + "emissive.png" ),
                                     GL_RGB,
                                     GL_RGB ) );
  }


  // Load room1 table1 textures 
  // --------------------------
  if( this->_model_id == 7 )
  { 
    std::string texture_name( "" );
    
    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );
    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );
  }


  // Load bottle textures 
  // --------------------
  if( this->_model_id == 8 )
  { 
    std::string texture_name( "" );
    
    if( iMaterialIndex == 0 )
    {
      texture_name += "bottle_";
    }

    if( iMaterialIndex == 1 )
    {
      texture_name += "strap_";
    }

    if( iMaterialIndex == 2 )
    {
      texture_name += "bottle_";
    }

    if( iMaterialIndex == 3 )
    {
      texture_name += "strap_";
    }

    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );
    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );

  }


  // Load ball textures 
  // --------------------
  if( this->_model_id == 9 )
  { 
    std::string texture_name( "" );

    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );

  }


  // Load box bag textures 
  // ---------------------
  if( this->_model_id == 10 )
  { 
    std::string texture_name( "" );

    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );

  }


  // Load chest textures 
  // -------------------
  if( this->_model_id == 11 )
  { 
    std::string texture_name( "" );

    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );

  }


  // Load sofa textures 
  // ------------------
  if( this->_model_id == 12 )
  { 
    std::string texture_name( "" );
    
    if( iMaterialIndex == 0 )
    {
      return textures;
    }

    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );
  }


  // Load sack textures 
  // --------------------
  if( this->_model_id == 13 )
  { 
    std::string texture_name( "" );
    
    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );
  }


  // Load room1_table2 textures 
  // --------------------------
  if( this->_model_id == 14 )
  { 
    std::string texture_name( "" );
    
    if( iMaterialIndex == 0 )
    {
      texture_name += "cable_";
    }

    if( iMaterialIndex == 1 )
    {
      texture_name += "table_";
    }

    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );
  }


  // Load book textures 
  // ------------------
  if( this->_model_id == 15 )
  { 
    std::string texture_name( "" );
    
    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );
  }


  // Load radio textures 
  // -------------------
  if( this->_model_id == 16 )
  { 
    std::string texture_name( "" );
    
    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );
  }


  // Load screen textures 
  // --------------------
  if( this->_model_id == 17 )
  { 
    std::string texture_name( "" );

    if( iMaterialIndex == 1 )
    {
      // albedo
      textures.push_back( LoadTexture( "uTextureAlbedo",
                                       std::string( "albedo.png" ),
                                       GL_RGB,
                                       GL_RGB ) );

      // normal
      textures.push_back( LoadTexture( "uTextureNormal",
                                       std::string( "normal.png" ),
                                       GL_RGB,
                                       GL_RGB ) );
      // AO
      textures.push_back( LoadTexture( "uTextureAO",
                                       std::string( "AO.png" ),
                                       GL_R8,
                                       GL_RED ) );

      // roughness 
      textures.push_back( LoadTexture( "uTextureRoughness",
                                       std::string( "roughness.png" ),
                                       GL_R8,
                                       GL_RED ) );

      // metalness
      textures.push_back( LoadTexture( "uTextureMetalness",
                                       std::string( "metalness.png" ),
                                       GL_R8,
                                       GL_RED ) );
    }
    
    if( iMaterialIndex == 0 )
    {
      // albedo
      textures.push_back( LoadTexture( "uTextureAlbedo",
                                       std::string( "albedo2.png" ),
                                       GL_RGB,
                                       GL_RGB ) );

      // normal
      textures.push_back( LoadTexture( "uTextureNormal",
                                       std::string( "normal2.png" ),
                                       GL_RGB,
                                       GL_RGB ) );
      // AO
      textures.push_back( LoadTexture( "uTextureAO",
                                       std::string( "AO2.png" ),
                                       GL_R8,
                                       GL_RED ) );

      // roughness 
      textures.push_back( LoadTexture( "uTextureRoughness",
                                       std::string( "roughness2.png" ),
                                       GL_R8,
                                       GL_RED ) );

      // metalness
      textures.push_back( LoadTexture( "uTextureMetalness",
                                       std::string( "metalness2.png" ),
                                       GL_R8,
                                       GL_RED ) );

      // emissive
      textures.push_back( LoadTexture( "uTextureEmissive",
                                       std::string( "emissive2.png" ),
                                       GL_RGB,
                                       GL_RGB ) );

      // opacity
      textures.push_back( LoadTexture( "uTextureOpacity",
                                       std::string( "opacity2.png" ),
                                       GL_R8,
                                       GL_RED ) );
      *oOpacityMap = true;

    }
  }


  // Load screen textures 
  // ----------------------
  if( this->_model_id == 18 )
  { 
    std::string texture_name( "" );

    if( iMaterialIndex == 1 )
    {
      // albedo
      textures.push_back( LoadTexture( "uTextureAlbedo",
                                       std::string( "glass_albedo.png" ),
                                       GL_RGB,
                                       GL_RGB ) );

      // normal
      textures.push_back( LoadTexture( "uTextureNormal",
                                       std::string( "glass_normal.png" ),
                                       GL_RGB,
                                       GL_RGB ) );
      // AO
      textures.push_back( LoadTexture( "uTextureAO",
                                       std::string( "glass_AO.png" ),
                                       GL_R8,
                                       GL_RED ) );

      // roughness 
      textures.push_back( LoadTexture( "uTextureRoughness",
                                       std::string( "glass_roughness.png" ),
                                       GL_R8,
                                       GL_RED ) );

      // metalness
      textures.push_back( LoadTexture( "uTextureMetalness",
                                       std::string( "glass_metalness.png" ),
                                       GL_R8,
                                       GL_RED ) );

      // opacity
      textures.push_back( LoadTexture( "uTextureOpacity",
                                       std::string( "glass_opacity.png" ),
                                       GL_R8,
                                       GL_RED ) );
      *oOpacityMap = true;
    }

    if( iMaterialIndex == 2 )
    {
      // albedo
      textures.push_back( LoadTexture( "uTextureAlbedo",
                                       std::string( "bike_albedo.png" ),
                                       GL_RGB,
                                       GL_RGB ) );

      // normal
      textures.push_back( LoadTexture( "uTextureNormal",
                                       std::string( "bike_normal.png" ),
                                       GL_RGB,
                                       GL_RGB ) );
      // AO
      textures.push_back( LoadTexture( "uTextureAO",
                                       std::string( "bike_AO.png" ),
                                       GL_R8,
                                       GL_RED ) );

      // roughness 
      textures.push_back( LoadTexture( "uTextureRoughness",
                                       std::string( "bike_roughness.png" ),
                                       GL_R8,
                                       GL_RED ) );

      // metalness
      textures.push_back( LoadTexture( "uTextureMetalness",
                                       std::string( "bike_metalness.png" ),
                                       GL_R8,
                                       GL_RED ) );
    }

    if( iMaterialIndex == 0 )
    {
      // albedo
      textures.push_back( LoadTexture( "uTextureAlbedo",
                                       std::string( "albedo.png" ),
                                       GL_RGB,
                                       GL_RGB ) );

      // normal
      textures.push_back( LoadTexture( "uTextureNormal",
                                       std::string( "normal.png" ),
                                       GL_RGB,
                                       GL_RGB ) );
      // AO
      textures.push_back( LoadTexture( "uTextureAO",
                                       std::string( "glass_AO.png" ),
                                       GL_R8,
                                       GL_RED ) );

      // roughness 
      textures.push_back( LoadTexture( "uTextureRoughness",
                                       std::string( "roughness.png" ),
                                       GL_R8,
                                       GL_RED ) );

      // metalness
      textures.push_back( LoadTexture( "uTextureMetalness",
                                       std::string( "metalness.png" ),
                                       GL_R8,
                                       GL_RED ) );
    }
  }


  // Load pilar textures 
  // -------------------
  if( this->_model_id == 19 )
  { 
    std::string texture_name( "" );
    
    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );
  }


  // Load scanner textures 
  // ---------------------
  if( this->_model_id == 20 )
  { 
    std::string texture_name( "" );
    
    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // emissive
    textures.push_back( LoadTexture( "uTextureEmissive",
                                     std::string( "emissive.png" ),
                                     GL_RGB,
                                     GL_RGB ) );
  }
  

  // Load room2_table1 textures 
  // --------------------------
  if( this->_model_id == 21 )
  { 
    std::string texture_name( "" );
    
    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );
  }


  // Load mask textures 
  // ------------------
  if( this->_model_id == 22 )
  { 
    std::string texture_name( "" );
    
    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
                                     GL_R8,
                                     GL_RED ) );
  }


  // Load arm textures 
  // -----------------
  if( this->_model_id == 23 )
  { 
    std::string texture_name( "" );
    
    // albedo
    textures.push_back( LoadTexture( "uTextureAlbedo",
                                     std::string( texture_name + "albedo.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // normal
    textures.push_back( LoadTexture( "uTextureNormal",
                                     std::string( texture_name + "normal.png" ),
                                     GL_RGB,
                                     GL_RGB ) );

    // AO
    textures.push_back( LoadTexture( "uTextureAO",
                                     std::string( texture_name + "AO.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // roughness 
    textures.push_back( LoadTexture( "uTextureRoughness",
                                     std::string( texture_name + "roughness.png" ),
                                     GL_R8,
                                     GL_RED ) );

    // metalness
    textures.push_back( LoadTexture( "uTextureMetalness",
                                     std::string( texture_name + "metalness.png" ),
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
  glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso );

  SDL_Surface * t = NULL;

  unsigned int textureID;
  glGenTextures( 1, &textureID );
  t = IMG_Load( iTexturePath.c_str() );
	  
  if( !t )
  {
  	std::cout << "Loading image fail => image null" << std::endl
  						<< "Path : " << iTexturePath << std::endl;
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

void Model::SetScene( Scene * iScene )
{
  _scene = iScene;
}

Scene * Model::GetScene()
{
  return _scene;
}
