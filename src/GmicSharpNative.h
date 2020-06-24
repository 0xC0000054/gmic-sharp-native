/*
 #
 #  File        : GmicSharpNative.h
 #                ( C++ header file )
 #
 #  Description : The native support library for gmic-sharp.
 #
 #  Copyright   : Nicholas Hayes
 #                ( https://github.com/0xC0000054 )
 #
 #  Licenses    : This file is 'dual-licensed', you have to choose one
 #                of the two licenses below to apply.
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
*/

#ifndef GMICSHARPNATIVE_H
#define GMICSHARPNATIVE_H

#ifdef _WIN32
  #ifdef GMICSHARPNATIVE_EXPORTS
    #ifdef __GNUC__
      #define DLL_EXPORT __attribute__ ((dllexport))
    #else
      #define DLL_EXPORT __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define DLL_EXPORT __attribute__ ((dllimport))
    #else
      #define DLL_EXPORT __declspec(dllimport)
    #endif
  #endif

  #ifdef __GNUC__
    #define GSN_API __attribute__ ((cdecl))
  #else
    #define GSN_API __cdecl
  #endif
#else
#define DLL_EXPORT
#define GSN_API __attribute__ ((cdecl))
#endif // _WIN32

class GmicImageList;

enum class GmicStatus
{
    Ok = 0,
    InvalidParameter,
    OutOfMemory,
    UnknownImageFormat,
    GmicError,
    GmicResourcePathInitFailed,
    GmicUnsupportedChannelCount,
    ImageListIndexOutOfRange,
    UnknownError
};

// G'MIC assumes the image data uses the range of [0, 255]
enum class ImageFormat
{
    // 8-bit grayscale with no alpha channel
    Gray8 = 0,
    // 8-bit grayscale with an 8-bit alpha channel
    GrayAlpha88,
    // Opaque 24-bit color using the RGB format, 8 bits per component
    Rgb888,
    // 32-bit color using the RGBA format, 8 bits per component
    Rgba8888
};

struct GmicErrorInfo
{
    char commandName[256];
    char errorMessage[256];
};

struct GmicOptions
{
    const char* commandLine;
    const char* customResourcePath;
    const char* hostName;
    float* progress;
    bool* abort;
};

struct GmicImageListPixelData
{
    union
    {
        float* red;
        float* gray;
    };
    float* green;
    float* blue;
    float* alpha;
};

struct GmicImageListImageData
{
    unsigned int width;
    unsigned int height;
    GmicImageListPixelData pixels;
    ImageFormat format;
};

extern "C" DLL_EXPORT void GSN_API GetLibraryVersion(int* major, int* minor, int* patch);

extern "C" DLL_EXPORT GmicImageList* GSN_API GmicImageListCreate();

extern "C" DLL_EXPORT void GSN_API GmicImageListDestroy(GmicImageList* list);

extern "C" DLL_EXPORT void GSN_API GmicImageListClear(GmicImageList * list);

extern "C" DLL_EXPORT unsigned int GSN_API GmicImageListGetCount(GmicImageList* list);

extern "C" DLL_EXPORT GmicStatus GSN_API GmicImageListGetImageData(
    GmicImageList* list,
    unsigned int index,
    GmicImageListImageData* data);


extern "C" DLL_EXPORT GmicStatus GSN_API GmicImageListAdd(
    GmicImageList* list,
    unsigned int width,
    unsigned int height,
    ImageFormat format,
    const char* name,
    GmicImageListPixelData* data);

extern "C" DLL_EXPORT GmicStatus GSN_API RunGmic(
    GmicImageList* images,
    GmicOptions* options,
    GmicErrorInfo* errorInfo);

#endif // GMICSHARPNATIVE_H
