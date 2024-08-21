#include "GustoDebug.h"

#include <iostream>

#include <visp3/core/vpConfig.h>
#include <optional>

#include <visp3/core/vpIoTools.h>
#include <visp3/detection/vpDetectorDNNOpenCV.h>
#include <visp3/gui/vpDisplayGDI.h>
#include <visp3/gui/vpDisplayOpenCV.h>
#include <visp3/gui/vpDisplayX.h>
#include <visp3/dnn_tracker/vpMegaPose.h>
#include <visp3/dnn_tracker/vpMegaPoseTracker.h>
#include <visp3/io/vpJsonArgumentParser.h>

#include <nlohmann/json.hpp>

#include <opencv2/videoio.hpp>

static vpImage<unsigned char> m_I; //!< Internal image updated using Visp_ImageUchar_SetFromColor32Array().
static vpCameraParameters m_cam; //!< Internal camera parameters updated using Visp_CameraParameters_Init().

unsigned width = 640, height = 480;
vpCameraParameters cam;
std::string videoDevice = "0";
std::string megaposeAddress = "127.0.0.1";
unsigned megaposePort = 5555;
int refinerIterations = 1, coarseNumSamples = 576;
double reinitThreshold = 0.2;

std::string detectorModelPath = "/media/sombrali/HDD1/3d_object_detection/visp/script/dataset_generator/yolov7/runs/train/yolo7x_640_480_20240414_001_3objects_combined/weights/best.onnx", detectorConfig = "none";
std::string detectorFramework = "onnx", detectorTypeString = "yolov7";
std::string objectName = "cube";
std::vector<std::string> labels = { "cube" };
float detectorMeanR = 0.f, detectorMeanG = 0.f, detectorMeanB = 0.f;
float detectorConfidenceThreshold = 0.65f, detectorNmsThreshold = 0.5f, detectorFilterThreshold = -0.25f;
float detectorScaleFactor = 0.0039f;
bool  detectorSwapRB = false;


void Gusto_MegaPoseServer_Init1()
{
  std::shared_ptr<vpMegaPose> megapose;
  try {
    std::cout << "debug1" << std::endl;

    megapose = std::make_shared<vpMegaPose>(megaposeAddress, megaposePort, cam, height, width);
  }
  catch (...) {
    std::cout << "Could not connect to Megapose server at " << megaposeAddress << std::endl;
    throw vpException(vpException::ioError, "Could not connect to Megapose server at " + megaposeAddress + " on port " + std::to_string(megaposePort));
  }
  std::cout << "debug2" << std::endl;
  vpMegaPoseTracker megaposeTracker(megapose, objectName, refinerIterations);
  megapose->setCoarseNumSamples(coarseNumSamples);
  const std::vector<std::string> allObjects = megapose->getObjectNames();

  if (std::find(allObjects.begin(), allObjects.end(), objectName) == allObjects.end()) {
    throw vpException(vpException::badValue, "Object " + objectName + " is not known by the Megapose server!");
  }
  std::future<vpMegaPoseEstimate> trackerFuture;

  bool Need_Reinit = true;
  bool overlayModel = true;
  std::cout << "debug3" << std::endl;
  bool has_track_future = false;
  
  if (has_track_future && trackerFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
    std::cout << "diu" << std::endl;
  }

}

int main()
{
  int argc = 3;
  const char *argv[3] = { "Debug", "--config", "/media/sombrali/HDD1/3d_object_detection/visp/visp-build/tutorial/tracking/dnn/data/gusto_webcam.json" };


/*
Unsolved issues:
- Megapose Init
/usr/bin/ld: CMakeFiles/GustoDebug.dir/GustoDebug.cpp.o: in function `Gusto_MegaPoseServer_Init1()':
GustoDebug.cpp:(.text+0x177f): undefined reference to `vpMegaPose::vpMegaPose(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, int, vpCameraParameters const&, unsigned int, unsigned int)'
/usr/bin/ld: CMakeFiles/GustoDebug.dir/GustoDebug.cpp.o: in function `std::_Sp_counted_ptr_inplace<vpMegaPose, std::allocator<vpMegaPose>, (__gnu_cxx::_Lock_policy)2>::_M_dispose()':
GustoDebug.cpp:(.text._ZNSt23_Sp_counted_ptr_inplaceI10vpMegaPoseSaIS0_ELN9__gnu_cxx12_Lock_policyE2EE10_M_disposeEv[_ZNSt23_Sp_counted_ptr_inplaceI10vpMegaPoseSaIS0_ELN9__gnu_cxx12_Lock_policyE2EE10_M_disposeEv]+0x9): undefined reference to `vpMegaPose::~vpMegaPose()'
collect2: error: ld returned 1 exit status
*/
  Gusto_MegaPoseServer_Init1();
  return 0;
}
