# Changelog

## [1.1.0] - 2025-xx-xx future release
### Added
- `curling::version()` — a new public function that returns the current version of the library.

### Changed
- Refactored several private methods within classes to improve code maintainability and readability. Moved to 'detail' namespace.
- No anonymous namespace in .cpp, as the Makefile will take care of building header only library (as well as doing the deb shared library package, as done previously). Anonymous namespace in header-only libs is not ideal. Moved to 'detail' namespace.
- These changes do not affect the public API or user-facing behavior.

---

## [1.0.0] - Initial Release
- First stable release.
