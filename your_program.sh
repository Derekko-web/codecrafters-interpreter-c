#!/bin/sh
#
# Use this script to run your program LOCALLY.
#
# Note: Changing this script WILL NOT affect how CodeCrafters runs your program.
#
# Learn more: https://codecrafters.io/program-interface

set -e # Exit early if any commands fail

# Copied from .codecrafters/compile.sh
#
# - Edit this to change how your program compiles locally
# - Edit .codecrafters/compile.sh to change how your program compiles remotely
(
  cd "$(dirname "$0")" # Ensure compile steps are run within the repository directory
  BUILD_DIR=build-local
  CMAKE_ARGS="-B ${BUILD_DIR} -S ."

  if [ -n "${VCPKG_ROOT:-}" ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
  fi

  cmake $CMAKE_ARGS
  cmake --build "./${BUILD_DIR}"
)

# Copied from .codecrafters/run.sh
#
# - Edit this to change how your program runs locally
# - Edit .codecrafters/run.sh to change how your program runs remotely
exec $(dirname "$0")/build-local/interpreter "$@"
