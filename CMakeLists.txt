#
#  File        : CMakeLists.txt
#                ( cmake-based build system configuration file )
#
#  Basic usage :
#
#                 - cmake .
#                 - make
#
#                ( https://github.com/0xC0000054/gmic-sharp-native )
#
#  Author      : Nicholas Hayes
#                ( https://github.com/0xC0000054 )
#
#                Based on work by:
#
#                Sander Knopper
#                ( https://github.com/saknopper )
#
#                Ștefan Talpalaru
#                ( https://github.com/stefantalpalaru )
#
#  Licenses    : This file is 'dual-licensed', you have to choose one
#                 of the two licenses below to apply.
#
#                CeCILL-C
#                The CeCILL-C license is close to the GNU LGPL.
#                ( http://cecill.info/licences/Licence_CeCILL-C_V1-en.html )
#
#            or  CeCILL v2.1
#                The CeCILL license is compatible with the GNU GPL.
#                ( http://cecill.info/licences/Licence_CeCILL_V2.1-en.html )
#
#  This software is governed either by the CeCILL or the CeCILL-C license
#  under French law and abiding by the rules of distribution of free software.
#  You can  use, modify and or redistribute the software under the terms of
#  the CeCILL or CeCILL-C licenses as circulated by CEA, CNRS and INRIA
#  at the following URL: "http://cecill.info".
#
#  As a counterpart to the access to the source code and  rights to copy,
#  modify and redistribute granted by the license, users are provided only
#  with a limited warranty  and the software's author,  the holder of the
#  economic rights,  and the successive licensors  have only  limited
#  liability.
#
#  In this respect, the user's attention is drawn to the risks associated
#  with loading,  using,  modifying and/or developing or reproducing the
#  software by the user in light of its specific status of free software,
#  that may mean  that it is complicated to manipulate,  and  that  also
#  therefore means  that it is reserved for developers  and  experienced
#  professionals having in-depth computer knowledge. Users are therefore
#  encouraged to load and test the software's suitability as regards their
#  requirements in conditions enabling the security of their systems and/or
#  data to be ensured and,  more generally, to use and operate it in the
#  same conditions as regards security.
#
#  The fact that you are presently reading this means that you have had
#  knowledge of the CeCILL and CeCILL-C licenses and that you accept its terms.
#

cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED ON)

project(gmicsharpnative CXX C)

find_package(PkgConfig)
include(FeatureSummary)
include(GNUInstallDirs)

# options controlling the build process
option(ENABLE_CURL "Add support for curl" ON)
option(ENABLE_X "Add support for X11" ON)
option(ENABLE_FFMPEG "Add support for FFMpeg" ON)
option(ENABLE_FFTW "Add support for FFTW" ON)
option(ENABLE_GRAPHICSMAGICK "Add support for GrahicsMagick" OFF)
option(ENABLE_JPEG "Add support for handling images in Jpeg format" ON)
option(ENABLE_OPENCV "Add support for OpenCV" ON)
option(ENABLE_OPENEXR "Add support for handling images in EXR format" ON)
option(ENABLE_OPENMP "Add support for parallel processing" ON)
option(ENABLE_PNG "Add support for handling images in PNG format" ON)
option(ENABLE_TIFF "Add support for handling images in Tiff format" ON) 
option(ENABLE_ZLIB "Add support for data compression via Zlib" ON)
option(ENABLE_LTO "Enable -flto (Link Time Optimizer) on gcc and clang" OFF)
option(USE_STATIC_LINKING "Statically link to dependencies" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (USE_STATIC_LINKING)
  if(WIN32)
    list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .lib .a)
  else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
  endif()
  message("Using static linking")
endif()

# compile flags
set(CMAKE_POSITION_INDEPENDENT_CODE True)

add_definitions(-Dgmic_build -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_abort -Dcimg_appname=\"gmic\")
if(APPLE)
  add_definitions(-mmacosx-version-min=10.8 -stdlib=libc++ -Wno-error=c++11-narrowing -Wc++11-extensions -fpermissive)
else()
  add_definitions(-Wno-error=narrowing -fno-ipa-sra -fpermissive)
endif()

if(NOT "${PRERELEASE_TAG}" STREQUAL "")
  add_definitions(-Dgmic_prerelease="${PRERELEASE_TAG}")
endif()

set(GMIC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gmic CACHE PATH "The directory the contains the G'MIC repository")

if(EXISTS ${GMIC_PATH}/src/gmic.cpp)
  message("Found G'MIC repository")
else()
  message("Could not find G'MIC repository at " ${GMIC_PATH})
  message("")
  message("You should try:")
  message("")
  message("   git clone https://github.com/dtschump/gmic.git \"" ${GMIC_PATH} "\"")
  message(FATAL_ERROR "G'MIC repository not found")
endif()

# CImg.h header
if(NOT EXISTS ${GMIC_PATH}/src/CImg.h)
  file(DOWNLOAD https://github.com/dtschump/CImg/raw/master/CImg.h ${GMIC_PATH}/src/CImg.h)
  execute_process(COMMAND ${CMAKE_COMMAND} -E touch ${GMIC_PATH}/src/CImg.h)
endif()

# gmic_stdlib.h header
if(NOT EXISTS ${GMIC_PATH}/src/gmic_stdlib.h)
  file(DOWNLOAD https://gmic.eu/gmic_stdlib.h ${GMIC_PATH}/src/gmic_stdlib.h)
  execute_process(COMMAND ${CMAKE_COMMAND} -E touch ${GMIC_PATH}/src/gmic_stdlib.h)
endif()

include_directories(${GMIC_PATH}/src)
set(libGmicSharpNative_SRCS

${GMIC_PATH}/src/gmic.h
${GMIC_PATH}/src/gmic.cpp
src/GmicSharpNative.cpp
)

if(WIN32)
  ENABLE_LANGUAGE(RC)
  # When building with MinGW CMake inserts flags that cause windres to fail
  # So we force the resource compiler to ignore all flags
  if (MINGW)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff <DEFINES> <INCLUDES> <SOURCE> <OBJECT>")
  endif()
 
  add_library(GmicSharpNative SHARED ${libGmicSharpNative_SRCS} src/version.rc)
else()
  add_library(GmicSharpNative SHARED ${libGmicSharpNative_SRCS})
endif()

set(EXTRA_LIBRARIES)

# OpenMP support
if(ENABLE_OPENMP)
  find_package(OpenMP)
endif()
if(OpenMP_CXX_FOUND)
  add_definitions(-fopenmp -Dcimg_use_openmp)
  target_link_libraries(GmicSharpNative PRIVATE OpenMP::OpenMP_CXX)
endif()

# Zlib support
if(ENABLE_ZLIB)
  find_package(ZLIB)
endif()
if(ZLIB_FOUND)
  add_definitions(-Dcimg_use_zlib)
  include_directories(${ZLIB_INCLUDE_DIRS})
  target_link_libraries(GmicSharpNative PRIVATE ZLIB::ZLIB)
endif()

# Curl support
if(ENABLE_CURL)
  find_package(CURL)
endif()
if(CURL_FOUND)
  add_definitions(-Dcimg_use_curl)
  if (USE_STATIC_LINKING)
    add_definitions(-DCURL_STATICLIB)
  endif() 
  include_directories(${CURL_INCLUDE_DIRS})
  target_link_libraries(GmicSharpNative PRIVATE CURL::libcurl) 
endif()

if (WIN32)
  add_definitions(-Dcimg_display=2 -DPSAPI_VERSION=1)
  list(APPEND EXTRA_LIBRARIES "psapi")
else()
  # X11 support
  if(ENABLE_X)
    find_package(X11)
  endif()
  if(X11_FOUND)
    add_definitions(-Dcimg_display=1)
    include_directories(${X11_INCLUDE_DIR})
    target_link_libraries(GmicSharpNative PRIVATE ${X11_LIBRARIES})
  else()
    add_definitions(-Dcimg_display=0)
  endif()
  if(X11_XShm_FOUND)
    add_definitions(-Dcimg_use_xshm)
	include_directories(${X11_XShm_INCLUDE_PATH})
	target_link_libraries(GmicSharpNative PRIVATE ${X11_Xext_LIB})
  endif()
endif()

if(ENABLE_FFTW)
  pkg_check_modules(FFTW3 fftw3>=3.0)
endif()
if(FFTW3_FOUND)
  add_definitions(-Dcimg_use_fftw3)
  include_directories(${FFTW3_INCLUDE_DIRS})
  target_link_libraries(GmicSharpNative PRIVATE fftw3)

  find_library(FFTW3_THREADS_LIB fftw3_threads PATHS ${FFTW3_LIBRARY_DIRS})
  if(FFTW3_THREADS_LIB STREQUAL "FFTW3_THREADS_LIB-NOTFOUND")
    add_definitions(-Dcimg_use_fftw3_singlethread)
  else()
    target_link_libraries(GmicSharpNative PRIVATE fftw3_threads)
  endif()
endif()

if(ENABLE_OPENCV)
  pkg_check_modules(OPENCV opencv)
endif()
if(OPENCV_FOUND)
  add_definitions(-Dcimg_use_opencv)
  include_directories(${OPENCV_INCLUDE_DIRS})
  target_link_libraries(GmicSharpNative PRIVATE OPENCV::opencv)
endif()

if(ENABLE_GRAPHICSMAGICK)
  pkg_check_modules(GRAPHICSMAGICK GraphicsMagick++)
endif()
if(GRAPHICSMAGICK_FOUND)
  add_definitions(-Dcimg_use_magick)
  include_directories(${GRAPHICSMAGICK_INCLUDE_DIRS})
  target_link_libraries(GmicSharpNative PRIVATE GRAPHICSMAGICK::GraphicsMagick++)
endif()

if(ENABLE_TIFF)
  find_package(TIFF)
endif()
if(TIFF_FOUND)
  add_definitions(-Dcimg_use_tiff)
  include_directories(${TIFF_INCLUDE_DIRS})
  target_link_libraries(GmicSharpNative PRIVATE TIFF::TIFF)
endif()

if(ENABLE_PNG)
  find_package(PNG)
endif()
if(PNG_FOUND)
  add_definitions(-Dcimg_use_png)
  include_directories(${PNG_INCLUDE_DIRS})
  target_link_libraries(GmicSharpNative PRIVATE PNG::PNG)
endif()

if(ENABLE_JPEG)
  find_package(JPEG)
endif()
if(JPEG_FOUND)
  add_definitions(-Dcimg_use_jpeg)
  include_directories(${JPEG_INCLUDE_DIRS})
  target_link_libraries(GmicSharpNative PRIVATE JPEG::JPEG)
endif()

if(ENABLE_OPENEXR)
  pkg_check_modules(OPENEXR OpenEXR)
endif()
if(OPENEXR_FOUND)
  add_definitions(-Dcimg_use_openexr)
  include_directories(${OPENEXR_INCLUDE_DIRS})
  target_link_libraries(GmicSharpNative PRIVATE OpenEXR)
endif()

if (ENABLE_LTO AND (CMAKE_COMPILER_IS_GNUCC OR (CMAKE_CXX_COMPILER_IS STREQUAL "Clang")))
  if (WIN32)
    message("Link Time Optimizer is not supported on Windows.")
  else()
    add_definitions(-flto)
    list(APPEND EXTRA_LIBRARIES "-flto")
  endif()
endif()

if (WIN32)
  add_definitions(-D_WIN32 -DGMICSHARPNATIVE_EXPORTS)
endif()

add_custom_target(gmic_extra_headers DEPENDS ${GMIC_PATH}/src/CImg.h ${GMIC_PATH}/src/gmic_stdlib.h)
set(CMAKE_CXX_FLAGS_DEBUG "-g -ansi -Wall -Wextra -pedantic -Dcimg_verbosity=3 ${COMPILE_FLAGS} -Og")
set(CMAKE_CXX_FLAGS_RELEASE "${COMPILE_FLAGS} -Ofast")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g ${COMPILE_FLAGS} -Ofast")

if(MINGW)
  list(APPEND EXTRA_LIBRARIES "-Wl,--stack,16777216")
else()
  list(APPEND EXTRA_LIBRARIES "-lpthread")
endif()

add_dependencies(GmicSharpNative gmic_extra_headers)

if(USE_STATIC_LINKING)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")  
endif()
target_link_libraries(GmicSharpNative PRIVATE ${EXTRA_LIBRARIES})

install(TARGETS GmicSharpNative RUNTIME DESTINATION bin LIBRARY DESTINATION lib)

feature_summary(WHAT ALL)
