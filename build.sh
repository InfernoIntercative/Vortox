#!/bin/bash

clear

rm -rf CMakeCache.txt CMakeFiles/

cmake .

if make VERBOSE=1; then
  ./bin/XylonEngine "$@"
else
  echo "Compilation failed. The binary will not be executed."
  exit 1
fi
