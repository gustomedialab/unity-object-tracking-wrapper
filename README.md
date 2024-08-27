## MACOS Build

### Install prerequisites
```
brew install cmake git
echo "export PATH=/usr/local/bin:$PATH" >> ~/.bash_profile
source ~/.bash_profile
```

### build
```
# brew would install opencv >= 4.10, so we don't need to build from source
brew install opencv glog lapack eigen libdc1394 zbar
# this is a 3rd-party lib for json which can be used as message transfer between client and server. We need this to build 'visp-dnn-trakcer'.
brew install nlohmann-json

git clone https://github.com/lagadic/visp.git

mkdir -p visp-build
cd visp-build
cmake ../visp
make -j$(sysctl -n hw.logicalcpu)


echo "export VISP_DIR=$PWD" >> ~/.bash_profile
source ~/.bash_profile


cmake ../vispWrapper
make
make install
```

## For Gusto Dev:
- C#/Unity side
  - unityProject/Assets/Scripts/Script_ar.cs
- C++ side
  - vispWrapper/GustoUnityWrapper.h
  - vispWrapper/GustoUnityWrapper.cpp
## Developement of a wrapper between ViSP and Unity engine

### Introduction

This project contains:
- a C++ wrapper between ViSP and Unity. This wrapper available in `vispWrapper` folder needs to be linked with [ViSP](https://visp.inria.fr).
- a Unity project available in `unityProject` folder that shows how to use this wrapper to build a demo to illustrate:
  - augmented reality displaying a virtual cube over an AprilTag
  - generic model-based tracking of a cube with an AprilTag glued on one cube face.

This project is compatible with Ubuntu, MacOS and Windows platforms.
