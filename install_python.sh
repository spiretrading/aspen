#!/bin/bash
python_directory=$(python3 -m site --user-site)
cp Libraries/Release/aspen.so $python_directory
