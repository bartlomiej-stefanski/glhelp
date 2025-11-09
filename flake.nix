{
  description = "Environment created for glhelp library development";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/release-25.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (
      system: let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            # Build utilites
            gnumake
            cmake
            ninja
            pkg-config
            include-what-you-use

            # Compilers
            gcc

            # Debugger and tools
            gdb
            gdb-dashboard
            clang-tools
            cppcheck
            valgrind
            doxygen

            # OpenGL library
            mesa
            mesa-demos
            libGL
            libGLU
            libGLU

            # Utility libraries
            glfw
            glm

            xorg.libX11
            xorg.libXi
            xorg.libXrandr
            xorg.libXext
            xorg.libXcursor
            xorg.libxcb
            xorg.libXinerama
          ];

          shellHook = ''
            echo "Welcome to your OpenGL development environment!"
            gcc --version | head -n 1
            clang --version | head -n 1
            glxinfo | grep "OpenGL version string" | head -n1
            CC=gcc
            CXX=g++
          '';
        };
      }
    );
}
