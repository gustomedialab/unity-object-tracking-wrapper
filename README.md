## For Gusto Dev:
- C#/Unity side
  - unityProject/Assets/Scripts/Script_ar.cs
- C++ side
  -vispWrapper/GustoUnityWrapper.h
  -vispWrapper/GustoUnityWrapper.cpp
## Developement of a wrapper between ViSP and Unity engine

### Introduction

This project contains:
- a C++ wrapper between ViSP and Unity. This wrapper available in `vispWrapper` folder needs to be linked with [ViSP](https://visp.inria.fr).
- a Unity project available in `unityProject` folder that shows how to use this wrapper to build a demo to illustrate:
  - augmented reality displaying a virtual cube over an AprilTag
  - generic model-based tracking of a cube with an AprilTag glued on one cube face.

This project is compatible with Ubuntu, MacOS and Windows platforms.

### Augmented Reality in Unity using ViSP

* The corresponding Unity scene is available in `unityProject/Assets/Scenes/scene_ar.unity`.
* On each new image AprilTag is detected and localized in 3D.
* A virtual red cube is projected in the scene over the tag thanks to the tag pose estimated with respect to the camera frame.
* Check the video demonstration on YouTube: https://youtu.be/iuD8syhNoGU
