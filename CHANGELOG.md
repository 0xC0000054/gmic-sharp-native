# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

* Allow callers to set a host application name.

### Changed

* An `ImageListIndexOutOfRange` error code is now used to indicate when the image list index is not valid.

### Fixed

* Catch all C++ exceptions in `GmicImageListCreate`.

## v0.6.0

### Added

* A Changelog

### Fixed

* An issue that prevented `GmicImageListCopyToOutput` from correctly converting 24-bit images to Bgr888x and Rgb888x.

## v0.5.0

### Added

First version

