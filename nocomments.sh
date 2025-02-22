#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <filename>"
    exit 1
fi

file="$1"

# use sed to remove comments
sed -E 's://.*$::g' "$file" | sed -E ':a;N;$!ba;s:/\*([^*]|\*[^/])*\*/::g' > "${file}.clean"

echo "Cleaned file saved as ${file}.clean"
