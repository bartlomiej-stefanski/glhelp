# README


## Dependencies

Nix dev environment is provided as a `flake.nix` file.

For other platforms, necessary libraries include:

- gcc supporting c++23
- CMake 3.24 or newer
- GLFW 3.3 or newer
- GLM 0.9 or newer
- GL 4.6
- X11 input-handling libraries


## Building

```bash
# To list all available presets run:
cmake --list-presets
# develop - preset for development
# minimal - minimal preset for shipping

cmake --preset=develop
cmake --build build
```
