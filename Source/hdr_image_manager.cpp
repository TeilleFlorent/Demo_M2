#include "hdr_image_manager.hpp"


//******************************************************************************
//**********  Class Clock  *****************************************************
//******************************************************************************

stbi_io_callbacks stbi__stdio_callbacks =
{
  HDRManager::stbi__stdio_read,
  HDRManager::stbi__stdio_skip,
  HDRManager::stbi__stdio_eof
};

HDRManager::HDRManager()
{
  _stbi__vertically_flip_on_load = 0;

  #ifndef STBI_NO_LINEAR
  _stbi__l2h_gamma = 2.2f;
  _stbi__l2h_scale = 1.0f;
  #endif

  _stbi__h2l_gamma_i = 1.0f / 2.2f;
  _stbi__h2l_scale_i = 1.0f;
}

void HDRManager::stbi__start_callbacks( stbi__context * s, stbi_io_callbacks * c, void * user )
{
  s->io = *c;
  s->io_user_data = user;
  s->buflen = sizeof( s->buffer_start );
  s->read_from_callbacks = 1;
  s->img_buffer_original = s->buffer_start;
  stbi__refill_buffer( s );
  s->img_buffer_original_end = s->img_buffer_end;
}

int HDRManager::stbi__stdio_read( void *  user, char * data, int size )
{
  return ( int ) fread( data, 1, size, ( FILE * ) user );
}

void HDRManager::stbi__stdio_skip( void * user, int n )
{
  fseek( ( FILE * ) user, n, SEEK_CUR );
}

int HDRManager::stbi__stdio_eof( void * user )
{
  return feof( ( FILE * ) user );
}

void HDRManager::stbi__start_file( stbi__context * s, FILE * f )
{
  stbi__start_callbacks( s, &stbi__stdio_callbacks, ( void * ) f );
}

void HDRManager::stbi__rewind( stbi__context * s )
{
  // conceptually rewind SHOULD rewind to the beginning of the stream,
  // but we just rewind to the beginning of the initial buffer, because
  // we only use it after doing 'test', which only ever looks at at most 92 bytes
  s->img_buffer = s->img_buffer_original;
  s->img_buffer_end = s->img_buffer_original_end;
}

int HDRManager::stbi__err_func( const char * str )
{
  _stbi__g_failure_reason = str;
  return 0;
}

void * HDRManager::stbi__malloc( size_t size )
{
  return STBI_MALLOC( size );
}

int HDRManager::stbi__addsizes_valid( int a, int b )
{
  if( b < 0 )
    return 0;
   // now 0 <= b <= INT_MAX, hence also
   // 0 <= INT_MAX - b <= INTMAX.
   // And "a + b <= INT_MAX" (which might overflow) is the
   // same as a <= INT_MAX - b (no overflow)
  return a <= INT_MAX - b;
}

int HDRManager::stbi__mul2sizes_valid( int a, int b )
{
  if( a < 0 || b < 0 )
    return 0;
  if( b == 0 )
    return 1; // mul-by-0 is always safe
  // portable way to check for no overflows in a*b
  return a <= INT_MAX / b;
}

int HDRManager::stbi__mad2sizes_valid( int a, int b, int add )
{
  return stbi__mul2sizes_valid( a, b ) && stbi__addsizes_valid( a * b, add );
}

int HDRManager::stbi__mad3sizes_valid( int a, int b, int c, int add )
{
  return stbi__mul2sizes_valid( a, b ) && stbi__mul2sizes_valid( a * b, c ) && stbi__addsizes_valid( a * b * c, add );
}

int HDRManager::stbi__mad4sizes_valid( int a, int b, int c, int d, int add )
{
  return stbi__mul2sizes_valid( a, b ) && stbi__mul2sizes_valid( a * b, c ) && stbi__mul2sizes_valid( a * b * c, d ) && stbi__addsizes_valid( a * b * c * d, add );
}

void * HDRManager::stbi__malloc_mad2( int a, int b, int add )
{
  if( !stbi__mad2sizes_valid( a, b, add ) )
    return NULL;
  return stbi__malloc( a * b + add );
}

void * HDRManager::stbi__malloc_mad3( int a, int b, int c, int add )
{
  if( !stbi__mad3sizes_valid( a, b, c, add ) )
    return NULL;
  return stbi__malloc( a * b * c + add );
}

void * HDRManager::stbi__malloc_mad4( int a, int b, int c, int d, int add )
{
  if( !stbi__mad4sizes_valid( a, b, c, d, add ) ) return NULL;
    return stbi__malloc( a * b * c * d + add );
}

void HDRManager::stbi_image_free( void * retval_from_stbi_load )
{
  STBI_FREE( retval_from_stbi_load );
}

void HDRManager::stbi_set_flip_vertically_on_load( int flag_true_if_should_flip )
{
  _stbi__vertically_flip_on_load = flag_true_if_should_flip;
}

void * HDRManager::stbi__load_main( stbi__context * s, int * x, int * y, int * comp, int req_comp, stbi__result_info * ri, int bpc )
{
  memset( ri, 0, sizeof( *ri ) ); // make sure it's initialized if we add new fields
  ri->bits_per_channel = 8; // default is 8 so most paths don't have to be changed
  ri->channel_order = STBI_ORDER_RGB; // all current input & output are this, but this is here so we can add BGR order
  ri->num_channels = 0;

  if( stbi__hdr_test( s ) )
  {
    float * hdr = stbi__hdr_load( s, x, y, comp, req_comp, ri );
    return stbi__hdr_to_ldr( hdr, *x, *y, req_comp ? req_comp : * comp );
  }
  return stbi__errpuc( "unknown image type", "Image not of any known type, or corrupt" );
}

stbi_uc * HDRManager::stbi__convert_16_to_8( stbi__uint16 * orig, int w, int h, int channels )
{
  int i;
  int img_len = w * h * channels;
  stbi_uc * reduced;

  reduced = ( stbi_uc * ) stbi__malloc( img_len );
  if( reduced == NULL )
    return stbi__errpuc( "outofmem", "Out of memory" );

  for( i = 0; i < img_len; ++i )
    reduced[ i ] = ( stbi_uc )( ( orig[ i ] >> 8 ) & 0xFF ); // top half of each byte is sufficient approx of 16->8 bit scaling

  STBI_FREE( orig );
  return reduced;
}

unsigned char * HDRManager::stbi__load_and_postprocess_8bit( stbi__context * s, int * x, int * y, int * comp, int req_comp )
{
  stbi__result_info ri;
  void * result = stbi__load_main( s, x, y, comp, req_comp, &ri, 8 );

  if( result == NULL )
    return NULL;

  if( ri.bits_per_channel != 8 )
  {
    STBI_ASSERT( ri.bits_per_channel == 16 );
    result = stbi__convert_16_to_8( ( stbi__uint16 * ) result, *x, *y, req_comp == 0 ? *comp : req_comp );
    ri.bits_per_channel = 8;
  }

  // @TODO: move stbi__convert_format to here

  if( _stbi__vertically_flip_on_load )
  {
    int w = *x, h = *y;
    int channels = req_comp ? req_comp : *comp;
    int row, col, z;
    stbi_uc * image = ( stbi_uc * ) result;

    // @OPTIMIZE: use a bigger temp buffer and memcpy multiple pixels at once
    for( row = 0; row < (h>>1); row++ )
    {
      for( col = 0; col < w; col++ )
      {
        for( z = 0; z < channels; z++ )
        {
          stbi_uc temp = image[ ( row * w + col ) * channels + z ];
          image[ ( row * w + col ) * channels + z ] = image[ ( ( h - row - 1 ) * w + col ) * channels + z ];
          image[ ( ( h - row - 1 ) * w + col ) * channels + z ] = temp;
        }
      }
    }
  }
  return ( unsigned char * ) result;
}

void HDRManager::stbi__float_postprocess( float * result, int * x, int * y, int * comp, int req_comp )
{
  if( _stbi__vertically_flip_on_load && result != NULL )
  {
    int w = *x, h = *y;
    int depth = req_comp ? req_comp : *comp;
    int row, col, z;
    float temp;

    // @OPTIMIZE: use a bigger temp buffer and memcpy multiple pixels at once
    for( row = 0; row < ( h >> 1 ); row++ )
    {
      for( col = 0; col < w; col++ )
      {
        for( z = 0; z < depth; z++ )
        {
          temp = result[ ( row * w + col ) * depth + z ];
          result[ ( row * w + col ) * depth + z ] = result[ ( ( h - row - 1 ) * w + col ) * depth + z ];
          result[ ( ( h - row - 1 ) * w + col ) * depth + z ] = temp;
        }
      }
    }
  }
}

#ifndef STBI_NO_STDIO
FILE * HDRManager::stbi__fopen( char const * filename, char const * mode )
{
  FILE * f;
  #if defined( _MSC_VER ) && _MSC_VER >= 1400
    if( 0 != fopen_s( &f, filename, mode ) )
      f=0;
  #else
    f = fopen( filename, mode );
  #endif
    return f;
}
#endif //!STBI_NO_STDIO

#ifndef STBI_NO_LINEAR
float * HDRManager::stbi__loadf_main( stbi__context * s, int * x, int * y, int * comp, int req_comp )
{
  unsigned char *data;
  if( stbi__hdr_test( s ) )
  {
    stbi__result_info ri;
    float * hdr_data = stbi__hdr_load( s, x, y, comp, req_comp, &ri );
    if( hdr_data )
      stbi__float_postprocess( hdr_data, x, y, comp, req_comp );
    return hdr_data;
  }
  data = stbi__load_and_postprocess_8bit( s, x, y, comp, req_comp );
  if( data )
    return stbi__ldr_to_hdr( data, *x, *y, req_comp ? req_comp : *comp );
  return stbi__errpf( "unknown image type", "Image not of any known type, or corrupt" );
}

#ifndef STBI_NO_STDIO
float * HDRManager::stbi_loadf( char const * filename, int * x, int * y, int * comp, int req_comp )
{
  float * result;
  FILE * f = stbi__fopen( filename, "rb" );
  if( !f )
    return stbi__errpf( "can't fopen", "Unable to open file" );
  result = stbi_loadf_from_file( f, x, y, comp, req_comp );
  fclose( f );
  return result;
}

float * HDRManager::stbi_loadf_from_file( FILE * f, int * x, int * y, int * comp, int req_comp )
{
  stbi__context s;
  stbi__start_file( &s, f );
  return stbi__loadf_main( &s, x, y, comp, req_comp );
}

#endif // !STBI_NO_STDIO
#endif // !STBI_NO_LINEAR

void HDRManager::stbi__refill_buffer( stbi__context * s )
{
  int n = ( s->io.read )( s->io_user_data, ( char * )s->buffer_start, s->buflen );
  if( n == 0 )
  {
    // at end of file, treat same as if from memory, but need to handle case
    // where s->img_buffer isn't pointing to safe memory, e.g. 0-byte file
    s->read_from_callbacks = 0;
    s->img_buffer = s->buffer_start;
    s->img_buffer_end = s->buffer_start + 1;
    *s->img_buffer = 0;
  }
  else
  {
    s->img_buffer = s->buffer_start;
    s->img_buffer_end = s->buffer_start + n;
  }
}

stbi_uc HDRManager::stbi__get8( stbi__context * s )
{
  if( s->img_buffer < s->img_buffer_end )
    return *s->img_buffer++;
  if( s->read_from_callbacks )
  {
    stbi__refill_buffer( s );
    return *s->img_buffer++;
  }
  return 0;
}

int HDRManager::stbi__at_eof( stbi__context * s )
{
  if( s->io.read )
  {
    if( !( s->io.eof )( s->io_user_data ) )
      return 0;
    // if feof() is true, check if buffer = end
    // special case: we've only got the special 0 character at the end
    if( s->read_from_callbacks == 0 )
      return 1;
  }
  return s->img_buffer >= s->img_buffer_end;
}

int HDRManager::stbi__getn( stbi__context * s, stbi_uc * buffer, int n )
{
  if( s->io.read )
  {
    int blen = ( int )( s->img_buffer_end - s->img_buffer );
    if( blen < n )
    {
      int res, count;
      memcpy( buffer, s->img_buffer, blen );
      count = ( s->io.read )( s->io_user_data, ( char * )buffer + blen, n - blen );
      res = ( count == ( n - blen ) );
      s->img_buffer = s->img_buffer_end;
      return res;
    }
  }

  if( s->img_buffer + n <= s->img_buffer_end )
  {
    memcpy( buffer, s->img_buffer, n );
    s->img_buffer += n;
    return 1;
  }
  else return 0;
}

#ifndef STBI_NO_LINEAR
float * HDRManager::stbi__ldr_to_hdr( stbi_uc * data, int x, int y, int comp )
{
  int i, k, n;
  float * output;
  if( !data )
    return NULL;
  output = ( float * ) HDRManager::stbi__malloc_mad4( x, y, comp, sizeof( float ), 0 );
  if( output == NULL )
  { 
    STBI_FREE( data );
    return stbi__errpf( "outofmem", "Out of memory" );
  }
  // compute number of non-alpha components
  if( comp & 1 )
  {
    n = comp; 
  }
  else
  {
    n = comp-1;
  }

  for( i = 0; i < x * y; ++i )
  {
    for( k = 0; k < n; ++k )
    {
      output[ i * comp + k ] = ( float )( pow( data[ i * comp + k ] / 255.0f, _stbi__l2h_gamma ) * _stbi__l2h_scale );
    }
    if( k < comp )
      output[ i * comp + k ] = data[ i * comp + k ] / 255.0f;
  }
  STBI_FREE( data );
  return output;
}
#endif

stbi_uc * HDRManager::stbi__hdr_to_ldr( float * data, int x, int y, int comp )
{
  int i, k, n;
  stbi_uc * output;
  if( !data )
    return NULL;
  output = ( stbi_uc * )stbi__malloc_mad3( x, y, comp, 0 );
  if( output == NULL )
  { 
    STBI_FREE( data );
    return stbi__errpuc( "outofmem", "Out of memory" );
  }
  // compute number of non-alpha components
  if( comp & 1 )
  {
    n = comp;
  }
  else
  { 
    n = comp - 1;
  }

  for( i = 0; i < x * y; ++i )
  {
    for( k = 0; k < n; ++k )
    {
      float z = ( float )pow( data[ i * comp + k ] * _stbi__h2l_scale_i, _stbi__h2l_gamma_i ) * 255 + 0.5f;
      if( z < 0 )
        z = 0;
      if( z > 255 )
        z = 255;
      output[ i * comp + k ] = ( stbi_uc ) stbi__float2int( z );
    }
    if( k < comp )
    {
      float z = data[ i * comp + k ] * 255 + 0.5f;
      if( z < 0 )
        z = 0;
      if( z > 255 )
        z = 255;
      output[ i * comp + k ] = ( stbi_uc ) stbi__float2int( z );
    }
  }
  STBI_FREE( data );
  return output;
}

int HDRManager::stbi__hdr_test_core( stbi__context * s, const char * signature )
{
  int i;
  for( i = 0; signature[ i ]; ++i )
  {
    if( stbi__get8( s ) != signature[ i ] )
    {
      return 0;
    }
  }
  stbi__rewind( s );
  return 1;
}

int HDRManager::stbi__hdr_test( stbi__context * s )
{
  int r = stbi__hdr_test_core( s, "#?RADIANCE\n" );
  stbi__rewind( s );
  if( !r )
  {
    r = stbi__hdr_test_core( s, "#?RGBE\n" );
    stbi__rewind( s );
  }
  return r;
}

char * HDRManager::stbi__hdr_gettoken( stbi__context * z, char * buffer )
{
  int len= 0;
  char c = '\0';

  c = ( char )stbi__get8( z );

  while( !stbi__at_eof( z ) && c != '\n' )
  {
    buffer[ len++ ] = c;
    if( len == STBI__HDR_BUFLEN - 1 )
    {
      // flush to end of line
      while( !stbi__at_eof( z ) && stbi__get8( z ) != '\n' )
      {
        ;
      }
      break;
    }
    c = ( char ) stbi__get8( z );
  }

  buffer[ len ] = 0;
  return buffer;
}

void HDRManager::stbi__hdr_convert( float * output, stbi_uc * input, int req_comp )
{
  if( input[ 3 ] != 0 )
  {
    float f1;
    // Exponent
    f1 = ( float )ldexp( 1.0f, input[ 3 ] - ( int )( 128 + 8 ) );
    if( req_comp <= 2 )
    {
      output[ 0 ] = ( input[ 0 ] + input[ 1 ] + input[ 2 ] ) * f1 / 3;
    }
    else
    {
      output[ 0 ] = input[ 0 ] * f1;
      output[ 1 ] = input[ 1 ] * f1;
      output[ 2 ] = input[ 2 ] * f1;
    }
    if( req_comp == 2 )
      output[ 1 ] = 1;
    if( req_comp == 4 )
      output[ 3 ] = 1;
  }
  else
  {
    switch( req_comp )
    {
      case 4: 
        output[ 3 ] = 1; /* fallthrough */
      
      case 3: 
        output[ 0 ] = output[ 1 ] = output[ 2 ] = 0;
        break;
      
      case 2:
        output[ 1 ] = 1; /* fallthrough */
      
      case 1:
        output[ 0 ] = 0;
        break;
    }
  }
}

float * HDRManager::stbi__hdr_load( stbi__context * s, int * x, int * y, int * comp, int req_comp, stbi__result_info * ri )
{
  char buffer[ STBI__HDR_BUFLEN ];
  char * token;
  int valid = 0;
  int width, height;
  stbi_uc * scanline;
  float * hdr_data;
  int len;
  unsigned char count, value;
  int i, j, k, c1,c2, z;
  const char * headerToken;
  STBI_NOTUSED( ri );

  // Check identifier
  headerToken = stbi__hdr_gettoken( s, buffer );
  if( strcmp( headerToken, "#?RADIANCE" ) != 0 && strcmp( headerToken, "#?RGBE" ) != 0 )
    return stbi__errpf( "not HDR", "Corrupt HDR image" );

  // Parse header
  for(;;)
  {
    token = stbi__hdr_gettoken( s,buffer );
    if( token[ 0 ] == 0 )
      break;
    if( strcmp( token, "FORMAT=32-bit_rle_rgbe" ) == 0 )
      valid = 1;
  }

  if( !valid )
    return stbi__errpf( "unsupported format", "Unsupported HDR format" );

  // Parse width and height
  // can't use sscanf() if we're not using stdio!
  token = stbi__hdr_gettoken( s, buffer );
  if( strncmp( token, "-Y ", 3 ) )
    return stbi__errpf( "unsupported data layout", "Unsupported HDR format" );
  token += 3;
  height = ( int )strtol( token, &token, 10 );
  while( *token == ' ' )
  {
    ++token;
  }
  if( strncmp( token, "+X ", 3 ) )
    return stbi__errpf( "unsupported data layout", "Unsupported HDR format" );
  token += 3;
  width = ( int )strtol( token, NULL, 10 );

  *x = width;
  *y = height;

  if( comp )
    *comp = 3;
  if( req_comp == 0 )
    req_comp = 3;

  if( !stbi__mad4sizes_valid( width, height, req_comp, sizeof( float ), 0) )
    return stbi__errpf( "too large", "HDR image is too large" );

  // Read data
  hdr_data = ( float * )
  stbi__malloc_mad4( width, height, req_comp, sizeof( float ), 0 );
  if( !hdr_data )
    return stbi__errpf( "outofmem", "Out of memory" );

  // Load image data
  // image data is stored as some number of sca
  if( width < 8 || width >= 32768 )
  {
    // Read flat data
    for( j = 0; j < height; ++j )
    {
      for( i = 0; i < width; ++i )
      {
        stbi_uc rgbe[ 4 ];
        main_decode_loop:
        stbi__getn( s, rgbe, 4 );
        stbi__hdr_convert( hdr_data + j * width * req_comp + i * req_comp, rgbe, req_comp );
      }
    }
  }
  else
  {
    // Read RLE-encoded data
    scanline = NULL;

    for( j = 0; j < height; ++j )
    {
      c1 = stbi__get8( s );
      c2 = stbi__get8( s );
      len = stbi__get8(s);
      if( c1 != 2 || c2 != 2 || ( len & 0x80 ) )
      {
        // not run-length encoded, so we have to actually use THIS data as a decoded
        // pixel (note this can't be a valid pixel--one of RGB must be >= 128)
        stbi_uc rgbe[ 4 ];
        rgbe[ 0 ] = ( stbi_uc ) c1;
        rgbe[ 1 ] = ( stbi_uc ) c2;
        rgbe[ 2 ] = ( stbi_uc ) len;
        rgbe[ 3 ] = ( stbi_uc ) stbi__get8( s );
        stbi__hdr_convert( hdr_data, rgbe, req_comp );
        i = 1;
        j = 0;
        STBI_FREE( scanline );
        goto main_decode_loop; // yes, this makes no sense
      }
      len <<= 8;
      len |= stbi__get8( s );
      if( len != width )
      {
        STBI_FREE( hdr_data );
        STBI_FREE( scanline );
        return stbi__errpf( "invalid decoded scanline length", "corrupt HDR" );
      }
      if( scanline == NULL )
      {
        scanline = ( stbi_uc * )stbi__malloc_mad2( width, 4, 0 );
        if( !scanline )
        {
          STBI_FREE( hdr_data );
          return stbi__errpf( "outofmem", "Out of memory" );
        }
      }

      for( k = 0; k < 4; ++k )
      {
        int nleft;
        i = 0;
        while( ( nleft = width - i ) > 0 )
        {
          count = stbi__get8( s );
          if( count > 128 )
          {
            // Run
            value = stbi__get8( s );
            count -= 128;
            if( count > nleft )
            { 
              STBI_FREE( hdr_data );
              STBI_FREE( scanline );
              return stbi__errpf( "corrupt", "bad RLE data in HDR" );
            }
            for( z = 0; z < count; ++z )
            {
              scanline[i++ * 4 + k] = value;
            }
          }
          else
          {
            // Dump
            if( count > nleft )
            { 
              STBI_FREE( hdr_data );
              STBI_FREE( scanline );
              return stbi__errpf( "corrupt", "bad RLE data in HDR" );
            }
            for( z = 0; z < count; ++z )
            {
              scanline[ i++ * 4 + k ] = stbi__get8( s );
            }
          }
        }
      }
      for( i = 0; i < width; ++i )
      {
        stbi__hdr_convert( hdr_data + ( j * width + i ) * req_comp, scanline + i * 4, req_comp );
      }
    }
    if( scanline )
      STBI_FREE( scanline );
  }
  return hdr_data;
}
