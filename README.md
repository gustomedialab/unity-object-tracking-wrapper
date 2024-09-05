## For Gusto Dev:
- C#/Unity side
  - unityProject/Assets/Scripts/gusto_tracking_example.cs
- C++ side
  - vispWrapper/GustoUnityWrapper.h
  - vispWrapper/GustoUnityWrapper.cpp

## Unity-Wrapper Build (all platforms)
```
git clone https://github.com/Sombraa711/Cpp-Unity-Binding?tab=readme-ov-file
cd Cpp-Unity-Binding
mkdir vispWrapper-build
cd vispWrapper-build
cmake ../vispWrapper
make 
make install
```

## Developement of a wrapper between ViSP and Unity engine
### Introduction

This project contains:
- a C++ wrapper between ViSP and Unity. This wrapper available in `vispWrapper` folder needs to be linked with [ViSP](https://visp.inria.fr).
- a Unity project available in `unityProject` folder that shows how to use this wrapper to build a demo to illustrate:
  - augmented reality displaying a virtual cube over an AprilTag
  - generic model-based tracking of a cube with an AprilTag glued on one cube face.

This project is compatible with Ubuntu, MacOS and Windows platforms.
