#!/bin/bash

clear

if make; then
  ./build/bin/Vortox "$@"
else
  echo "Compilation failed. Do you want to debug? (y/n)"
  read -r answer
  if [[ "$answer" == "y" || "$answer" == "Y" ]]; then
    gdb ./build/bin/Vortox
  else
    echo "Exiting..."
    exit 1
  fi
fi
