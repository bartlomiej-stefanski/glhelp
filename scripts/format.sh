#!/usr/bin/env bash

PROJ_DIR="$( dirname -- "${BASH_SOURCE[0]}" )/.."

dirs=("$PROJ_DIR/src" "$PROJ_DIR/include" "$PROJ_DIR/examples")

clang-format --files=<(find "${dirs[@]}" -type f \( -iname "*.h" -o -iname "*.cpp" -o -iname "*.hpp" \) ) -i
