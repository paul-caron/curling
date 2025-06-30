# Changelog

## [1.2.0] - 2025-06-29
### Added
- static assertion / compile time check that Request cannot be copied.
- Request::setRawOption(CURLoption opt, <T> value). For the advanced users.
- helper script 'inline.sh' to help with header-only library.

### Changed
- The error thrown after failed curl_perform for a more detailed error message that includes error code and url.
- Changed deb control file dependency libcurl4-openssl-dev to libcurl4-dev, which now allows users to choose which ssl backend they want to use.


## [1.1.0] - 2025-06-28
### Added
- `curling::version()` — a new public function that returns the current version of the library.
- added setHttpVersion method, to be able to specify Http 1.1, 2 or 3
- examples - example codes folder with different cases including how to multithread and how to add a progress bar.

### Changed
- Refactored several private methods within classes to improve code maintainability and readability. Moved to 'detail' namespace.
- No anonymous namespace in .cpp, as the Makefile will take care of building header only library (as well as doing the deb shared library package, as done previously). Anonymous namespace in header-only libs is not ideal. Moved to 'detail' namespace.
- These changes do not affect the public API or user-facing behavior.
- Makefile was changed to add option to build header-only library.

---

## [1.0.0] - Initial Release
- First stable release.
