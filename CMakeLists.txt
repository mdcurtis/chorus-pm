project( squeezeslave )

cmake_minimum_required(VERSION 2.6)

# Put all binaries at top level
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

option( FRONTEND_ENABLE "Build the squeezeslave binary" ON )
option( SLIMAUDIO_TREMOR "Use the Tremor fixed-point OGG decoder (for embedded systems) instead of the standard one" OFF )
option( SLIMPROTO_DEBUG "Enable debug logging" OFF )
option( SLIMPROTO_ZONES "Enable multi-zone support" OFF )
option( SLIMPROTO_RENICE "Allow http and output threads to alter their thread priorities" OFF )
option( BUILD_SHARED_LIBS "" ON )

find_package( PkgConfig )

function( pkg_check_extended prefix package includeCheck libraryCheck mandatory )
	pkg_check_modules( ${prefix} ${package} QUIET )

	if( NOT ${prefix}_FOUND )
		find_path( ${prefix}_INCLUDE_DIRS ${includeCheck} )
		find_library( ${prefix}_LIBRARIES ${libraryCheck} )

		if( ${prefix}_INCLUDE_DIRS AND ${prefix}_LIBRARIES )
			set( ${prefix}_FOUND TRUE PARENT_SCOPE )
			set( ${prefix}_FOUND TRUE )
		endif()
	endif()

	if( ${prefix}_FOUND )
		message( STATUS "Found ${package} ${${prefix}_VERSION}" )
	elseif( ${mandatory} STREQUAL REQUIRED )
		message( SEND_ERROR "Unable to find required package/library ${package}.  Please install this or manually point ${prefix}_INCLUDE_DIRS and ${prefix}_LIBRARIES to the location of ${includeCheck} header and the ${libraryCheck} library, respectively.")
	endif()
endfunction()

pkg_check_extended( MAD mad mad.h mad REQUIRED )
pkg_check_extended( OGG ogg ogg/ogg.h ogg REQUIRED )
pkg_check_extended( VORBIS vorbis vorbis/vorbis.h vorbis REQUIRED )
if( SLIMAUDIO_TREMOR )
	pkg_check_extended( VORBISIDEC vorbisidec vorbis/ivorbisfile.h vorbisidec REQUIRED )
else()
	pkg_check_extended( VORBISFILE vorbisfile vorbis/vorbisfile.h vorbisfile REQUIRED )
endif()
pkg_check_extended( FLAC flac FLAC/all.h flac REQUIRED )
pkg_check_extended( PORTAUDIO portaudio-2.0 portaudio.h portaudio REQUIRED )

# FIXME: allow for building the special minimal versions of PortAudio
# TODO: is this really necessary?  How much smaller does it make the binary?
set( SLIMPROTO_PADEV TRUE )

# Fix bug in some versions of the FLAC pkg-config file which incorrectly add FLAC to the end of the include path
get_filename_component( FLAC_LEAFNAME ${FLAC_INCLUDE_DIRS} NAME )
if( FLAC_LEAFNAME STREQUAL FLAC )
	get_filename_component( FLAC_PATH ${FLAC_INCLUDE_DIRS} PATH )
	set( FLAC_INCLUDE_DIRS ${FLAC_PATH} FORCE )
endif()

include_directories( ${MAD_INCLUDE_DIRS} )
include_directories( ${OGG_INCLUDE_DIRS} )
include_directories( ${VORBIS_INCLUDE_DIRS} )
if( SLIMAUDIO_TREMOR )
	include_directories( ${VORBISIDEC_INCLUDE_DIRS} )
else()
	include_directories( ${VORBISFILE_INCLUDE_DIRS} )
endif()
include_directories( ${FLAC_INCLUDE_DIRS} )
include_directories( ${PORTAUDIO_INCLUDE_DIRS} )

file( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src/slimproto )
configure_file( src/slimproto/config.h.in ${CMAKE_BINARY_DIR}/src/slimproto/config.h )

include_directories( ${CMAKE_BINARY_DIR}/src )
include_directories( ${CMAKE_SOURCE_DIR}/src )

add_library( slimproto  
	src/slimaudio/slimaudio_http.c 
	src/slimaudio/slimaudio_decoder.c
	src/slimaudio/slimaudio_decoder_flac.c
	src/slimaudio/slimaudio_decoder_mad.c
	src/slimaudio/slimaudio_decoder_vorbis.c
	src/slimaudio/slimaudio_decoder_pcm.c
	src/slimaudio/slimaudio_output.c
	src/slimaudio/slimaudio_buffer.c
	src/slimaudio/slimaudio.c
	src/slimproto/slimproto.c
	src/slimproto/renice.c )

target_link_libraries( slimproto ${MAD_LIBRARIES} ${OGG_LIBRARIES} ${VORBIS_LIBRARIES} ${FLAC_LIBRARIES} ${PORTAUDIO_LIBRARIES} )
if( SLIMAUDIO_TREMOR )
	target_link_libraries( slimproto ${VORBISIDEC_LIBRARIES} )
else()
	target_link_libraries( slimproto ${VORBISFILE_LIBRARIES} )
endif()

install( TARGETS slimproto LIBRARY DESTINATION lib RUNTIME DESTINATION lib ARCHIVE DESTINATION lib )
install( FILES src/slimaudio/slimaudio.h src/slimaudio/slimaudio_buffer.h DESTINATION include/slimaudio )
install( FILES src/slimproto/slimproto.h ${CMAKE_BINARY_DIR}/src/slimproto/config.h DESTINATION include/slimproto )

if( FRONTEND_ENABLE )
	add_subdirectory( src/squeezeslave )
	
endif()