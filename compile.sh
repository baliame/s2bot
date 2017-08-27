#!/usr/bin/env bash
git submodule update --init
cd s2client-api
gmfix=$(cat .gitmodules | sed 's/https:\/\/github.com\//git@github.com:/g')
echo $gmfix > .gitmodules
git submodule update --init
git checkout .gitmodules
if [ -d build ]; then rm -rf build; fi
mkdir build
cd build
cmake ../ -G "Visual Studio 15 Win64"
start libs.bat
