#!/bin/bash

INC_DIR="include/"
SOURCE_FILE="main.cpp"
OUTPUT_EXECUTABLE="nn-viz.exe"

if ! pacman -Qs raylib > /dev/null; then
    echo "Error: raylib is not installed. Please run 'sudo pacman -S raylib' to install it."
    exit 1
fi

INC="-I$INC_DIR"
LDFLAGS="-lraylib"
CXXFLAGS="-ggdb"

g++ $CXXFLAGS -o $OUTPUT_EXECUTABLE $INC $SOURCE_FILE $LDFLAGS

if [ $? -eq 0 ]; then
    echo "Build successful!"
else
    echo "Build failed."
fi

