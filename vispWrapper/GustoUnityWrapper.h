
#ifndef VISPUnity_h
#define VISPUnity_h

/*!
  \file
  \brief ViSPUnity plugin functions declaration.
 */
#include <visp3/visp_core.h>
#include <visp3/io/vpImageIo.h>
#include <visp3/blob/vpDot2.h>
#include <visp3/detection/vpDetectorAprilTag.h>
#include <visp3/mbt/vpMbGenericTracker.h>


#include <visp3/core/vpIoTools.h>
#include <visp3/detection/vpDetectorDNNOpenCV.h>
#include <visp3/gui/vpDisplayGDI.h>
#include <visp3/gui/vpDisplayOpenCV.h>
#include <visp3/gui/vpDisplayX.h>
#include <visp3/dnn_tracker/vpMegaPose.h>
#include <visp3/dnn_tracker/vpMegaPoseTracker.h>
#include <visp3/io/vpJsonArgumentParser.h>



#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <ios>

#  ifdef ViSPUnity_EXPORTS
#    define VISP_UNITY_EXPORT VISP_DLLEXPORT
#  else
#    define VISP_UNITY_EXPORT VISP_DLLIMPORT
#  endif

extern "C" {


  VISP_UNITY_EXPORT
    void Gusto_EnableDebugOption(bool _debug_option = true);

  VISP_UNITY_EXPORT
    void Gusto_CameraParameters_Init(double cam_px = 600., double cam_py = 600., double cam_u0 = 340., double cam_v0 = 240.);

  VISP_UNITY_EXPORT
    void Gusto_MegaPoseServer_Init();

  VISP_UNITY_EXPORT
    // void Gusto_Init(const std::string config_path);
    void Gusto_Init(const char *config_path);
  VISP_UNITY_EXPORT
    void Gusto_ImageUchar_SetFromColor32Array(unsigned char *bitmap, int height, int width);

  VISP_UNITY_EXPORT
    bool Gusto_Detection2D_Process(double *bbox_xywh, double *detection_time);
  // VISP_UNITY_EXPORT
  //   std::optional<vpRect> detectObjectForInitMegaposeDnn(vpDetectorDNNOpenCV &detector, const cv::Mat &I, const std::string &detectionLabel);

  VISP_UNITY_EXPORT
    bool Gusto_MegaPose_Tracking_Process(float *position, float *rotation);

  VISP_UNITY_EXPORT
    void Gusto_CppWrapper_MemoryFree();

      //Create a callback delegate
  typedef void(*FuncCallBack)(const char *message, int color, int size);
  static FuncCallBack callbackInstance = nullptr;
  VISP_UNITY_EXPORT
    void RegisterDebugCallback(FuncCallBack cb);
}
//Color Enum
enum class Color { Red, Green, Blue, Black, White, Yellow, Orange };

class  Debug
{
public:
  static void Log(const char *message, Color color = Color::Black);
  static void Log(const std::string message, Color color = Color::Black);
  static void Log(const int message, Color color = Color::Black);
  static void Log(const char message, Color color = Color::Black);
  static void Log(const float message, Color color = Color::Black);
  static void Log(const double message, Color color = Color::Black);
  static void Log(const bool message, Color color = Color::Black);
  static void Log(const vpHomogeneousMatrix message, Color color = Color::Black);
  static void Log(const vpTranslationVector message, Color color = Color::Black);
  static void Log(const vpRotationMatrix message, Color color = Color::Black);
private:
  static void send_log(const std::stringstream &ss, const Color &color);
};
#endif
