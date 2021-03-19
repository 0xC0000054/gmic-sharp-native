# gmic-sharp-native

The native support library for [gmic-sharp](https://github.com/0xC0000054/gmic-sharp).
This provides the native interface between gmic-sharp and [libgmic](https://github.com/dtschump/gmic).

## Building the library

### Linux

#### Building with the installed version of G'MIC

This version dynamically links to the installed version of the G'MIC library.

Assuming that you are building a 64-bit library with libgmic located in the system search path, you would run the following commands from the shell:  

`gcc -shared -fPIC src/GmicSharpNative.cpp -o bin/x64/libGmicSharpNative.so -lgmic -lpthread -lX11 -lstdc++ -lm`

#### Building with a local copy of G'MIC

This version uses the G'MIC source code when building the library.

First clone the G'MIC repository into `<root>/gmic`.

Next create a build directory and run CMake to setup the build environment.

`mkdir build && cd build`   
`cmake ..`

Once CMake has finished configuring  the library can be built by running `make`.

### Windows

#### Using MSYS2:

This is the preferred method as it allows all G'MIC features to be used.
Assuming that you are building the 64-bit DLL you would run the following commands from the MSYS2 64 shell:   

First create the build folder: `mkdir build64 && cd build64`
Second generate the MingGW makefiles: `cmake -G "MinGW Makefiles" ..`
Third build the library: `mingw32-make`

This should result in `libGmicSharpNative.dll` being generated in the `buil64` folder.

You will need to copy it and the native dependencies to the folder of your choice.
Assuming your target folder is the `bin/x64` folder in the GmicSharpNative root, you can use the following commands:

Copy `libGmicSharpNative.dll` to the `bin/x64` folder:

`cp ./libGmicSharpNative.dll ../bin/x64/libGmicSharpNative.dll`

Copy the required dependences from the `/mingw64/bin` folder:

`cd /mingw64/bin/ && cp libgcc_s_seh-1.dll libwinpthread-1.dll libgomp-1.dll libstdc++-6.dll libcurl-4.dll libbrotlidec.dll libbrotlicommon.dll libcrypto-1_1-x64.dll libidn2-0.dll libiconv-2.dll libintl-8.dll libunistring-2.dll libnghttp2-14.dll libpsl-5.dll libssh2-1.dll zlib1.dll libssl-1_1-x64.dll libzstd.dll libfftw3-3.dll libjpeg-8.dll libpng16-16.dll libtiff-5.dll liblzma-5.dll <GmicSharpNative_ROOT>/bin/x64`

The above list was taken from the Windows build instructions for the G'MIC CLI on the [G'MIC download page](https://gmic.eu/download.html#windows).
Depending on the library versions that you have installed there may be additional dependencies that are not listed above.

#### Using Visual Studio 2019:

This should only be used for debugging the `gmic-sharp` Managed <--> Native transitions.
The Visual C++ 2019 compiler does not currently support all of the features of the GCC build.

Turn off all CMake options except the static library and configure CMake to build G'MIC with Visual Studio.
Then open the `libgmicstatic` project, change the CRT runtime to static and build it.

Note that you may have to fix some parts of the G'MIC code to work with the Visual C++ compiler.

After building `libgmicstatic` open the project in the msvc folder.
Update the include and library paths, then you should be able to build the project.

## License

This project is dual-licensed under the terms of the either the [CeCILL v2.1](https://cecill.info/licences/Licence_CeCILL_V2.1-en.html) (GPL-compatible) or [CeCILL-C v1](https://cecill.info/licences/Licence_CeCILL-C_V1-en.html) (similar to the LGPL).  
Pick the one you want to use.

See [COPYING](COPYING) for more information.

This was done to match the licenses used by [libgmic](https://github.com/dtschump/gmic).