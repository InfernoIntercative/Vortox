#!/bin/bash

clear

if make; then
  ./bin/XylonEngine "$@"
else
  echo "Compilation failed. The binary will not be executed."
  exit 1
fi
