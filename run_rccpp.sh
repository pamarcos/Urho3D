#!/bin/bash

URHO3D_HOME=$(pwd)/build
URHO3D_HOME=$URHO3D_HOME $URHO3D_HOME/bin/Urho3DPlayer $1 -w -x 640 -y 480
