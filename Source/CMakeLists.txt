file( GLOB_RECURSE source_files *.cpp )

include_directories( Library )

add_executable( ${app_name} ${source_files} )

if( WIN32 )
	target_link_libraries( ${app_name} )
else()
	target_link_libraries( ${app_name} GL SDL2 GLEW SDL2_image SDL2_mixer assimp )
endif()