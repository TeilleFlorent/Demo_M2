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
  std::string vertex_code;
  std::string fragment_code;
  std::ifstream vertex_shader_file;
  std::ifstream fragment_shader_file;
  vertex_shader_file.exceptions( std::ifstream::badbit );
  fragment_shader_file.exceptions( std::ifstream::badbit );
  try
  {
    vertex_shader_file.open( iVertexPath );
    fragment_shader_file.open( iFragmentPath );
    std::stringstream vertex_shader_stream, fragment_shader_stream;
    vertex_shader_stream << vertex_shader_file.rdbuf();
    fragment_shader_stream << fragment_shader_file.rdbuf();
    vertex_shader_file.close();
    fragment_shader_file.close();
    vertex_code = vertex_shader_stream.str();
    fragment_code = fragment_shader_stream.str();
  }
  catch( std::ifstream::failure e )
  {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
  }

  const GLchar * vertex_shader_code = vertex_code.c_str();
  const GLchar * fragment_shader_code = fragment_code.c_str();
  unsigned int vertex, fragment;
  GLint success;
  GLchar infoLog[ 512 ];

  // Vertex shader
  vertex = glCreateShader( GL_VERTEX_SHADER );
  glShaderSource( vertex, 1, &vertex_shader_code, NULL );
  glCompileShader( vertex );
  glGetShaderiv( vertex, GL_COMPILE_STATUS, &success );
  if( !success )
  {
    glGetShaderInfoLog( vertex, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // Fragment shader
  fragment = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( fragment, 1, &fragment_shader_code, NULL );
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
  std::string vertex_code;
  std::string geo_code;
  std::string fragment_code;
  std::ifstream vertex_shader_file;
  std::ifstream geo_shader_file;
  std::ifstream fragment_shader_file;
  vertex_shader_file.exceptions(   std::ifstream::badbit );
  geo_shader_file.exceptions(      std::ifstream::badbit );
  fragment_shader_file.exceptions( std::ifstream::badbit );

  try
  {
    vertex_shader_file.open(   iVertexPath );
    geo_shader_file.open(      iGeometryPath );
    fragment_shader_file.open( iFragmentPath );
    std::stringstream vertex_shader_stream, geo_shader_stream, fragment_shader_stream;
    vertex_shader_stream   << vertex_shader_file.rdbuf();
    geo_shader_stream      << geo_shader_file.rdbuf();
    fragment_shader_stream << fragment_shader_file.rdbuf();
    vertex_shader_file.close();
    geo_shader_file.close();
    fragment_shader_file.close();
    vertex_code   = vertex_shader_stream.str();
    geo_code      = geo_shader_stream.str();
    fragment_code = fragment_shader_stream.str();
  }
  catch( std::ifstream::failure e )
  {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
  }

  const GLchar * vertex_shader_code   = vertex_code.c_str();
  const GLchar * geo_shader_code      = geo_code.c_str();
  const GLchar * fragment_shader_code = fragment_code.c_str();
  unsigned int vertex, geo, fragment;
  GLint success;
  GLchar infoLog[ 512 ];
  
  // Vertex shader
  vertex = glCreateShader( GL_VERTEX_SHADER );
  glShaderSource( vertex, 1, &vertex_shader_code, NULL );
  glCompileShader( vertex );
  glGetShaderiv( vertex, GL_COMPILE_STATUS, &success );
  if( !success )
  {
    glGetShaderInfoLog( vertex, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // Geometry shader
  geo = glCreateShader( GL_GEOMETRY_SHADER );
  glShaderSource( geo, 1, &geo_shader_code, NULL );
  glCompileShader( geo );
  glGetShaderiv( geo, GL_COMPILE_STATUS, &success );
  if( !success )
  {
    glGetShaderInfoLog( geo, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // Fragment shader
  fragment = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( fragment, 1, &fragment_shader_code, NULL );
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

void Shader::SetShaderTessellationPipeline( const char * iVertexPath,
                                            const char * iTessellationControlPath,
                                            const char * iTessellationEvaluationPath,
                                            const char * iFragmentPath )
{
  std::string vertex_code;
  std::string tess_control_code;
  std::string tess_eval_code;
  std::string fragment_code;
  std::ifstream vertex_shader_file;
  std::ifstream tess_control_shader_file;
  std::ifstream tess_eval_shader_file;
  std::ifstream fragment_shader_file; 
  vertex_shader_file.exceptions(       std::ifstream::badbit );
  tess_control_shader_file.exceptions( std::ifstream::badbit );
  tess_eval_shader_file.exceptions(    std::ifstream::badbit );
  fragment_shader_file.exceptions(     std::ifstream::badbit );

  try
  {
    vertex_shader_file.open(       iVertexPath );
    tess_control_shader_file.open( iTessellationControlPath );
    tess_eval_shader_file.open(    iTessellationEvaluationPath );
    fragment_shader_file.open(     iFragmentPath );
    std::stringstream vertex_shader_stream, tess_control_shader_stream, tess_eval_shader_stream, fragment_shader_stream;
    vertex_shader_stream       << vertex_shader_file.rdbuf();
    tess_control_shader_stream << tess_control_shader_file.rdbuf();
    tess_eval_shader_stream    << tess_eval_shader_file.rdbuf();
    fragment_shader_stream     << fragment_shader_file.rdbuf();
    vertex_shader_file.close();
    tess_control_shader_file.close();
    tess_eval_shader_file.close();
    fragment_shader_file.close();
    vertex_code       = vertex_shader_stream.str();
    tess_control_code = tess_control_shader_stream.str();
    tess_eval_code    = tess_eval_shader_stream.str();
    fragment_code     = fragment_shader_stream.str();
  }
  catch( std::ifstream::failure e )
  {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
  }

  const GLchar * vertex_shader_code       = vertex_code.c_str();
  const GLchar * tess_control_shader_code = tess_control_code.c_str();
  const GLchar * tess_eval_shader_code    = tess_eval_code.c_str();
  const GLchar * fragment_shader_code     = fragment_code.c_str();
  unsigned int vertex, tess_control, tess_eval, fragment;
  GLint success;
  GLchar infoLog[ 512 ];
  
  // Vertex shader
  vertex = glCreateShader( GL_VERTEX_SHADER );
  if( vertex == 0 )
  {
    fprintf( stderr, "Error creating shader type %d\n", GL_VERTEX_SHADER );
  }
  glShaderSource( vertex, 1, &vertex_shader_code, NULL );
  glCompileShader( vertex );
  glGetShaderiv( vertex, GL_COMPILE_STATUS, &success );
  if( !success )
  {
    glGetShaderInfoLog( vertex, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // Tess control shader
  tess_control = glCreateShader( GL_TESS_CONTROL_SHADER );
  if( tess_control == 0 )
  {
    fprintf( stderr, "Error creating shader type %d\n", GL_TESS_CONTROL_SHADER );
  }
  glShaderSource( tess_control, 1, &tess_control_shader_code, NULL );
  glCompileShader( tess_control );
  glGetShaderiv( tess_control, GL_COMPILE_STATUS, &success );
  if( !success )
  {
    glGetShaderInfoLog( tess_control, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::TESS CONTROL::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // Tess eval shader
  tess_eval = glCreateShader( GL_TESS_EVALUATION_SHADER );
  if( tess_eval == 0 )
  {
    fprintf( stderr, "Error creating shader type %d\n", GL_TESS_EVALUATION_SHADER );
  }
  glShaderSource( tess_eval, 1, &tess_eval_shader_code, NULL );
  glCompileShader( tess_eval );
  glGetShaderiv( tess_eval, GL_COMPILE_STATUS, &success );
  if( !success )
  {
    glGetShaderInfoLog( tess_eval, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::TESS CONTROL::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // Fragment shader
  fragment = glCreateShader( GL_FRAGMENT_SHADER );
  if( fragment == 0 )
  {
    fprintf( stderr, "Error creating shader type %d\n", GL_FRAGMENT_SHADER );
  }
  glShaderSource( fragment, 1, &fragment_shader_code, NULL );
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
  glAttachShader( this->_program, tess_control );
  glAttachShader( this->_program, tess_eval );
  glAttachShader( this->_program, fragment );

  // Link shader program
  glLinkProgram(  this->_program );
  glGetProgramiv( this->_program, GL_LINK_STATUS, &success );
  if( !success )
  {
    glGetProgramInfoLog( this->_program, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n\n" << infoLog << std::endl;
  }

  // Validate shader program
  glValidateProgram( this->_program );
  glGetProgramiv( this->_program, GL_VALIDATE_STATUS, &success );
  if( !success )
  { 
    glGetProgramInfoLog( this->_program, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::PROGRAM::VALIDATION_FAILED\n\n" << infoLog << std::endl;
  }

  // Free
  glDeleteShader( vertex );
  glDeleteShader( tess_control );
  glDeleteShader( tess_eval );
  glDeleteShader( fragment );
}

