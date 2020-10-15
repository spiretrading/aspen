#!/bin/bash
if [ "$1" == "" ]; then
  config="Release"
else
  config="$1"
fi
python_directory=$(python3 -m site --user-site)
cp "Libraries/$config/aspen.so" "$python_directory"
