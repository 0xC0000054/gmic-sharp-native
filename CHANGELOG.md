# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

* `name` and `nameLength` fields to the `GmicImageListImageData` structure.
  * This allows G'MIC to set the name of the output image.
  
### Changed

* Removed the bit-depth from the `ImageFormat` value names.  
* Renamed the `ImageFormat` `Rgba` value to `RgbAlpha`.  

### Removed

* The `GmicImageListClear` method.

## [v0.7.0](https://github.com/0xC0000054/gmic-sharp-native/compare/v0.6.0...v0.7.0) - 2020-07-10

### Added

* Allow callers to set a host application name.
* A `GetLibraryVersion` method.
* A `GrayAlpha88` value to the `NativeImageFormat` enumeration.
* A CMake configuration file.

### Changed

* An `ImageListIndexOutOfRange` error code is now used to indicate when the image list index is not valid.
* Support custom resource paths that use UTF-8.
* Callers now have to perform their own conversion to/from the G'MIC image format.
* Allow builds to include gmic.cpp

### Fixed

* Catch all C++ exceptions in `GmicImageListCreate`.

### Removed

* The `customUserPath` field in `GmicOptions`.
* The `Bgr888`, `Bgr888x`, `Bgra8888` and `Rgb888x` values from the `NativeImageFormat` enumeration.

## v0.6.0

### Added

* A Changelog

### Fixed

* An issue that prevented `GmicImageListCopyToOutput` from correctly converting 24-bit images to Bgr888x and Rgb888x.

## v0.5.0

### Added

First version
