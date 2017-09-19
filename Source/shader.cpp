#include "shader.hpp"


//******************************************************************************
//**********  Class Shader  ****************************************************
//******************************************************************************
  
Shader::Shader()
{

}

void Shader::Use() 
{ 
  glUseProgram( this->_program ); 
}

void Shader::SetShaderClassicPipeline( const GLchar * iVertexPath,
                                       const GLchar * iFragmentPath )
{
  std::string vertexCode;
  std::string fragmentCode;
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;
  vShaderFile.exceptions( std::ifstream::badbit );
  fShaderFile.exceptions( std::ifstream::badbit );
  try
  {
    vShaderFile.open( iVertexPath );
    fShaderFile.open( iFragmentPath );
    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    vShaderFile.close();
    fShaderFile.close();
    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();
  }
  catch( std::ifstream::failure e )
  {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
  }

  const GLchar * vShaderCode = vertexCode.c_str();
  const GLchar * fShaderCode = fragmentCode.c_str();
  GLuint vertex, fragment;
  GLint success;
  GLchar infoLog[ 512 ];

  // Vertex shader
  vertex = glCreateShader( GL_VERTEX_SHADER );
  glShaderSource( vertex, 1, &vShaderCode, NULL );
  glCompileShader( vertex );
  glGetShaderiv( vertex, GL_COMPILE_STATUS, &success );
  if( !success )
  {
    glGetShaderInfoLog( vertex, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // Fragment shader
  fragment = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( fragment, 1, &fShaderCode, NULL );
  glCompileShader( fragment );
  glGetShaderiv( fragment, GL_COMPILE_STATUS, &success );
  if( !success )
  {
    glGetShaderInfoLog( fragment, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // Create shader program
  this->_program = glCreateProgram();
  glAttachShader( this->_program, vertex );
  glAttachShader( this->_program, fragment );
  glLinkProgram( this->_program );
  glGetProgramiv( this->_program, GL_LINK_STATUS, &success );
  if( !success )
  {
    glGetProgramInfoLog( this->_program, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }
  glDeleteShader( vertex );
  glDeleteShader( fragment );
}

void Shader::SetShaderGeometryPipeline( const GLchar * iVertexPath,
                                        const GLchar * iGeometryPath,
                                        const GLchar * iFragmentPath )
{
  std::string vertexCode;
  std::string geoCode;
  std::string fragmentCode;
  std::ifstream vShaderFile;
  std::ifstream gShaderFile;
  std::ifstream fShaderFile;
  vShaderFile.exceptions( std::ifstream::badbit );
  gShaderFile.exceptions( std::ifstream::badbit );
  fShaderFile.exceptions( std::ifstream::badbit );
  try
  {
    vShaderFile.open( iVertexPath );
    gShaderFile.open( iGeometryPath );
    fShaderFile.open( iFragmentPath );
    std::stringstream vShaderStream, gShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    gShaderStream << gShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    vShaderFile.close();
    gShaderFile.close();
    fShaderFile.close();
    vertexCode = vShaderStream.str();
    geoCode = gShaderStream.str();
    fragmentCode = fShaderStream.str();
  }
  catch( std::ifstream::failure e )
  {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
  }
  const GLchar * vShaderCode = vertexCode.c_str();
  const GLchar * gShaderCode = geoCode.c_str();
  const GLchar * fShaderCode = fragmentCode.c_str();
  GLuint vertex, geo, fragment;
  GLint success;
  GLchar infoLog[ 512 ];
  
  // Vertex shader
  vertex = glCreateShader( GL_VERTEX_SHADER );
  glShaderSource( vertex, 1, &vShaderCode, NULL );
  glCompileShader( vertex );
  glGetShaderiv( vertex, GL_COMPILE_STATUS, &success );
  if( !success )
  {
    glGetShaderInfoLog( vertex, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // Geometry shader
  geo = glCreateShader( GL_GEOMETRY_SHADER );
  glShaderSource( geo, 1, &gShaderCode, NULL );
  glCompileShader( geo );
  glGetShaderiv( geo, GL_COMPILE_STATUS, &success );
  if( !success )
  {
    glGetShaderInfoLog( geo, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // Fragment shader
  fragment = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( fragment, 1, &fShaderCode, NULL );
  glCompileShader( fragment );
  glGetShaderiv( fragment, GL_COMPILE_STATUS, &success );
  if( !success )
  {
    glGetShaderInfoLog( fragment, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // Create shader program
  this->_program = glCreateProgram();
  glAttachShader( this->_program, vertex );
  glAttachShader( this->_program, geo );
  glAttachShader( this->_program, fragment );
  glLinkProgram( this->_program );
  glGetProgramiv( this->_program, GL_LINK_STATUS, &success );
  if( !success )
  {
    glGetProgramInfoLog( this->_program, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }
  glDeleteShader( vertex );
  glDeleteShader( geo );
  glDeleteShader( fragment );
}

