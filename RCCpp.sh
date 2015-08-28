#!/bin/bash

./cmake_clean.sh Build
rm -rf Build
./cmake_generic.sh Build -G "Unix Makefiles" -DURHO3D_PCH=0 -DCMAKE_BUILD_TYPE=Debug -DURHO3D_RCCPP=1 -DURHO3D_64BIT=1 -DURHO3D_LIB_TYPE=SHARED
