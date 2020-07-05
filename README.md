# gmic-sharp-native

The native support library for [gmic-sharp](https://github.com/0xC0000054/gmic-sharp).
This provides the native interface between gmic-sharp and [libgmic](https://github.com/dtschump/gmic).

## Building the library

### Using MSYS2:

This is the preferred method as it allows all G'MIC features to be used.
I build libgmic as static library with CURL, FFTW, JPEG, OpenMP, PNG and TIFF enabled.
The final DLL is statically linked to all of its non-OS dependencies.

Once libgmic is built change the MSYS shell to the `src` folder in the project root.

Assuming that you are building a statically-linked version of the 64-bit DLL with libgmic.a located in `<root>gmic/gcc64`, you would run the following commands from the MSYS2 64 shell:   

First compile the version resources:
`windres ./version.rc version.o`

Then compile the DLL:
`g++ -shared -DGMICSHARPNATIVE_EXPORTS -I../gmic/src ./GmicSharpNative.cpp ./version.o -o ../bin/x64/libGmicSharpNative.dll -L../gmic/gcc64 -Wl,--stack,16777216 -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32 -static -lgmic -lfftw3 -lcurl -ltiff -lz -lzstd -llzma -lgomp -ljpeg -lpng`

### Using Visual Studio 2019:

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