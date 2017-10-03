cmake_minimum_required( VERSION 3.2.0 )

set( app_name "Demo" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin )
set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_FLAGS "-O2 -g -lm" )

project( Demo )
subdirs( Source )