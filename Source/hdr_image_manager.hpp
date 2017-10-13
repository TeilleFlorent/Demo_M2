#ifndef INCLUDE_IMAGE_HDR_MANAGER_H
#define INCLUDE_IMAGE_HDR_MANAGER_H

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif // STBI_NO_STDIO

#if !defined( STBI_NO_LINEAR )
#include <math.h>  // ldexp
#endif

#define STBI__HDR_BUFLEN 2048

using namespace std;


//******************************************************************************
//**********  HDR Manager Typedefs  ********************************************
//******************************************************************************

typedef unsigned char stbi_uc;
typedef unsigned short stbi_us;

#ifdef _MSC_VER
typedef unsigned short stbi__uint16;
typedef   signed short stbi__int16;
typedef unsigned int   stbi__uint32;
typedef   signed int   stbi__int32;
#else
#include <stdint.h>
typedef uint16_t stbi__uint16;
typedef int16_t  stbi__int16;
typedef uint32_t stbi__uint32;
typedef int32_t  stbi__int32;
#endif


//******************************************************************************
//**********  HDR Manager Macros  **********************************************
//******************************************************************************

#ifndef STBI_ASSERT
#include <assert.h>
#define STBI_ASSERT( x ) assert( x )
#endif

#ifdef _MSC_VER
#define STBI_NOTUSED( v ) ( void )( v )
#else
#define STBI_NOTUSED( v ) ( void )sizeof( v )
#endif

#ifndef STBI_MALLOC
#define STBI_MALLOC( sz ) malloc( sz )
#define STBI_FREE( p ) free( p )
#endif

#ifdef STBI_NO_FAILURE_STRINGS
  #define stbi__err( x, y ) 0
#elif defined( STBI_FAILURE_USERMSG ) 
  #define stbi__err( x, y ) stbi__err_func( y )
#else
  #define stbi__err( x, y ) stbi__err_func( x )
#endif

#define stbi__errpf( x, y ) ( ( float * )( size_t ) ( stbi__err( x, y ) ? NULL : NULL ) )
#define stbi__errpuc( x, y ) ( ( unsigned char * )( size_t ) ( stbi__err( x, y ) ? NULL : NULL ) )

#define stbi__float2int( x ) ( ( int ) ( x ) )


//******************************************************************************
//**********  HDR Manager Structs/Enums  ***************************************
//******************************************************************************

typedef struct
{
  int   ( *read )  ( void * user, char * data, int size );   // fill 'data' with 'size' bytes.  return number of bytes actually read
  void  ( *skip )  ( void * user, int n );                   // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
  int   ( *eof )   ( void * user );                          // returns nonzero if we are at end of file/data
} stbi_io_callbacks;

// stbi__context struct and start_xxx functions
// stbi__context structure is our basic context used by all images, so it
// contains all the IO context, plus some basic image information
typedef struct
{
  stbi__uint32 img_x, img_y;
  int img_n, img_out_n;

  stbi_io_callbacks io;
  void * io_user_data;

  int read_from_callbacks;
  int buflen;
  stbi_uc buffer_start[ 128 ];

  stbi_uc * img_buffer, * img_buffer_end;
  stbi_uc * img_buffer_original, * img_buffer_original_end;
} stbi__context;

typedef struct
{
  int bits_per_channel;
  int num_channels;
  int channel_order;
} stbi__result_info;

enum
{
  STBI_ORDER_RGB,
  STBI_ORDER_BGR
};

enum
{
  STBI__SCAN_load = 0,
  STBI__SCAN_type,
  STBI__SCAN_header
};


//******************************************************************************
//**********  Class HDRManager  ************************************************
//******************************************************************************

class HDRManager
{

  public:


    // HDRManager functions
    // --------------------

    HDRManager();

    // float-per-channel interface
    float * stbi_loadf( char const * filename, int * x, int * y, int * channels_in_file, int desired_channels );

    float * stbi_loadf_from_file( FILE * f, int * x, int * y, int * channels_in_file, int desired_channels );

    // free the loaded image -- this is just free()
    void stbi_image_free( void * retval_from_stbi_load );

    // flip the image vertically, so the first pixel in the output array is the bottom left
    void stbi_set_flip_vertically_on_load( int flag_true_if_should_flip );

    ////   end header file   /////////////////////////////////////////////////////

    void stbi__refill_buffer( stbi__context * s );

    // initialize a callback-based context
    void stbi__start_callbacks( stbi__context * s, stbi_io_callbacks * c, void * user );

    static int stbi__stdio_read( void *  user, char * data, int size );

    static void stbi__stdio_skip( void * user, int n );

    static int stbi__stdio_eof( void * user );

    void stbi__start_file( stbi__context * s, FILE * f );

    void stbi__rewind( stbi__context * s );

    int stbi__hdr_test( stbi__context * s );

    float * stbi__hdr_load( stbi__context * s, int * x, int * y, int * comp, int req_comp, stbi__result_info * ri );

    int stbi__hdr_info( stbi__context * s, int * x, int * y, int * comp );

    int stbi__err_func( const char * str );

    void * stbi__malloc( size_t size );

    // return 1 if the sum is valid, 0 on overflow.
    // negative terms are considered invalid.
    int stbi__addsizes_valid( int a, int b );

    // returns 1 if the product is valid, 0 on overflow.
    // negative factors are considered invalid.
    int stbi__mul2sizes_valid( int a, int b );

    // returns 1 if "a*b + add" has no negative terms/factors and doesn't overflow
    int stbi__mad2sizes_valid( int a, int b, int add );

    // returns 1 if "a*b*c + add" has no negative terms/factors and doesn't overflow
    int stbi__mad3sizes_valid( int a, int b, int c, int add );

    // returns 1 if "a*b*c*d + add" has no negative terms/factors and doesn't overflow
    int stbi__mad4sizes_valid( int a, int b, int c, int d, int add );

    // mallocs with size overflow checking
    void * stbi__malloc_mad2( int a, int b, int add );

    void * stbi__malloc_mad3( int a, int b, int c, int add );

    void * stbi__malloc_mad4( int a, int b, int c, int d, int add );

    // stbi__err - error
    // stbi__errpf - error returning pointer to float
    // stbi__errpuc - error returning pointer to unsigned char

    float * stbi__ldr_to_hdr( stbi_uc * data, int x, int y, int comp );

    stbi_uc * stbi__hdr_to_ldr( float * data, int x, int y, int comp );

    void * stbi__load_main( stbi__context * s, int * x, int * y, int * comp, int req_comp, stbi__result_info * ri, int bpc );

    stbi_uc * stbi__convert_16_to_8( stbi__uint16 * orig, int w, int h, int channels );

    unsigned char * stbi__load_and_postprocess_8bit( stbi__context * s, int * x, int * y, int * comp, int req_comp );

    void stbi__float_postprocess( float * result, int * x, int * y, int * comp, int req_comp );

    FILE * stbi__fopen( char const * filename, char const * mode );

    float * stbi__loadf_main( stbi__context * s, int * x, int * y, int * comp, int req_comp );

    stbi_uc stbi__get8( stbi__context * s );

    int stbi__at_eof( stbi__context * s );

    int stbi__getn( stbi__context * s, stbi_uc * buffer, int n );

    int stbi__hdr_test_core( stbi__context * s, const char * signature );

    char * stbi__hdr_gettoken( stbi__context * z, char * buffer );

    void stbi__hdr_convert( float * output, stbi_uc * input, int req_comp );


    // HDRManager class members
    // ------------------------

    int _stbi__vertically_flip_on_load;

    const char * _stbi__g_failure_reason;

    float _stbi__l2h_gamma;
    float _stbi__l2h_scale;

    float _stbi__h2l_gamma_i;
    float _stbi__h2l_scale_i;

};

#endif // INCLUDE_IMAGE_HDR_MANAGER_H
