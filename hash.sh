#!/bin/bash

dir="src"

if [ ! -d "$dir" ]; then
  echo "[PANIC] Nop! The folder doesn't exist."
  exit 1
fi

find "$dir" -type f -exec sha256sum {} \; > correct_hash.TXT
