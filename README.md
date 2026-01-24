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

## Licesnes For Demo-Models

- "City" (https://skfb.ly/o6zrK) by SpatialNeglect is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
- "2023 Porsche 911 Dakar Rallye Design Package" (https://skfb.ly/ps6PD) by Ddiaz Design is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
- "Old Tree" (https://skfb.ly/6TvHn) by gelmi.com.br is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
