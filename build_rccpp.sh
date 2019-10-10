#!/bin/bash

cmake -H. -DURHO3D_RCCPP=1 -DURHO3D_LIB_TYPE=SHARED -DCMAKE_BUILD_TYPE=Debug -Bbuild -G Ninja
time cmake --build build
