## For Gusto Dev:
- C#/Unity side
  - unityProject/Assets/Scripts/gusto_tracking_example.cs
  - unityProject/Assets/GustoUnityWrapper.bundle # Engine For MACOS
  - unityProject/Assets/Weights/yolov7-tiny-20240821-3cls2.onnx # 2D Detection Model
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
