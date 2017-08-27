#!/usr/bin/env bash
if [ -z "$(which cmake 2>/dev/null)" ]; then echo "CMake is not installed or not in path."; exit 1; fi
git submodule update --init
cd s2client-api
sed -i'' 's/https:\/\/github.com\//git@github.com:/g' .gitmodules
git submodule update --init
git checkout .gitmodules
if [ -d build ]; then rm -rf build; fi
mkdir build
cd build
cmake ../ -G "Visual Studio 15 Win64"
if [ -z "$(which msbuild 2>/dev/null)" ]; then
	echo "MSBuild is not in path. Attempting to use VS2017 version."
	if stat '/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/MSBuild/15.0/Bin/MSBuild.exe' >/dev/null 2>&1; then
		'/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/MSBuild/15.0/Bin/MSBuild.exe' -m s2client-api.sln
	else
		echo "Could not stat MSBuild for VS2017. Aborting."
		exit 1
	fi
else
	msbuild -m s2client-api.sln
fi
cd ../..
start libs.bat
