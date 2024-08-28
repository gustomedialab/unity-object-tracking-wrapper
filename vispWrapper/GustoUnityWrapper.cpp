#include "GustoUnityWrapper.h"

#include <visp3/gui/vpDisplayGDI.h>
#include <visp3/gui/vpDisplayOpenCV.h>
#include <visp3/gui/vpDisplayX.h>

#include <nlohmann/json.hpp>

extern "C" {


  void Debug::Log(const vpHomogeneousMatrix message, Color color)
  {
    std::stringstream ss;
    ss << message;
    send_log(ss, color);
  }
  void Debug::Log(const vpTranslationVector message, Color color)
  {
    std::stringstream ss;
    ss << message;
    send_log(ss, color);
  }
  void Debug::Log(const vpRotationMatrix message, Color color)
  {
    std::stringstream ss;
    ss << message;
    send_log(ss, color);
  }
  //-------------------------------------------------------------------
  void  Debug::Log(const char *message, Color color)
  {
    if (callbackInstance != nullptr)
      callbackInstance(message, (int)color, (int)strlen(message));
  }

  void  Debug::Log(const std::string message, Color color)
  {
    const char *tmsg = message.c_str();
    if (callbackInstance != nullptr)
      callbackInstance(tmsg, (int)color, (int)strlen(tmsg));
  }

  void  Debug::Log(const int message, Color color)
  {
    std::stringstream ss;
    ss << message;
    send_log(ss, color);
  }

  void  Debug::Log(const char message, Color color)
  {
    std::stringstream ss;
    ss << message;
    send_log(ss, color);
  }

  void Debug::Log(const float message, Color color)
  {
    std::stringstream ss;
    ss << message;
    send_log(ss, color);
  }

  void  Debug::Log(const double message, Color color)
  {
    std::stringstream ss;
    ss << message;
    send_log(ss, color);
  }

  void Debug::Log(const bool message, Color color)
  {
    std::stringstream ss;
    if (message)
      ss << "true";
    else
      ss << "false";

    send_log(ss, color);
  }

  void Debug::send_log(const std::stringstream &ss, const Color &color)
  {
    const std::string tmp = ss.str();
    const char *tmsg = tmp.c_str();
    if (callbackInstance != nullptr)
      callbackInstance(tmsg, (int)color, (int)strlen(tmsg));
  }
  //-------------------------------------------------------------------

  //Create a callback delegate
  void RegisterDebugCallback(FuncCallBack cb)
  {
    callbackInstance = cb;
  }



  static bool m_debug_enable_display = true; //!< Flag used to enable/disable display associated to internal image m_I.
  static bool m_debug_display_is_initialized = false; //!< Flag used to know if display associated to internal image m_I is initialized.
  static vpDisplay *displayer = nullptr;


/*!
 * Global variables that are common.
 */
  cv::Mat frame;
  vpImage<vpRGBa> m_I; //!< Internal image updated using Visp_ImageUchar_SetFromColor32Array().
  static vpCameraParameters m_cam; //!< Internal camera parameters updated using Visp_CameraParameters_Init().

  unsigned width = 640, height = 480;
  vpCameraParameters cam;
  std::string videoDevice;
  std::string megaposeAddress;
  unsigned megaposePort;
  int refinerIterations = 1, coarseNumSamples = 576;
  double reinitThreshold = 0.2;

  std::string detectorModelPath = "./best.onnx", detectorConfig = "none";
  std::string detectorFramework = "onnx", detectorTypeString = "yolov7";
  std::string objectName = "cube";
  std::vector<std::string> labels = { "cube" };
  float detectorMeanR = 0.f, detectorMeanG = 0.f, detectorMeanB = 0.f;
  float detectorConfidenceThreshold = 0.65f, detectorNmsThreshold = 0.5f, detectorFilterThreshold = -0.25f;
  float detectorScaleFactor = 0.0039f;
  bool  detectorSwapRB = false;
  //! [Arguments]

  vpDetectorDNNOpenCV dnn;
  std::optional<vpRect> detection = std::nullopt;

  std::shared_ptr<vpMegaPose> megapose;
  bool megapose_initialized = false;
  std::future<vpMegaPoseEstimate> trackerFuture;

  std::vector<double> megaposeTimes;
  vpMegaPoseEstimate megaposeEstimate; // last Megapose estimation
  static vpMegaPoseTracker *megaposeTracker = nullptr;
  bool megaposeTracker_initialized = false;

  double megaposeStartTime = 0.0;
  vpRect lastDetection; // Last detection (initialization)
  bool initialized = false;
  bool tracking = false;
  bool callMegapose = true;
  vpImage<vpRGBa> overlayImage(height, width);

  // recycle functions
  // not only pointer we need to free, the global variables should be reset as well.
  void Gusto_CppWrapper_MemoryFree()
  {
    if (m_debug_display_is_initialized) {
      m_debug_display_is_initialized = false;
      delete displayer;
    }
    if (megaposeTracker_initialized) {
      megaposeTracker_initialized = false;
      // delete megaposeTracker; // sombra: I think it should be deleted but it would cause crash... need to check later
      megaposeTracker = nullptr;
    }
    if (megapose_initialized) {
      initialized = false;
      tracking = false;
      callMegapose = true;
      megapose_initialized = false;
      megapose->~vpMegaPose();
    }

    return;
  }

  void Gusto_ImageUchar_SetFromColor32Array(unsigned char *bitmap, int height, int width)
  {
    frame = cv::Mat(height, width, CV_8UC4, bitmap);
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    cv::flip(frame, frame, 0);

    // vpImageConvert::convert(m_I, frame);
    vpImageConvert::convert(frame, m_I);
    // cv::resize(frame, frame, cv::Size(width, height));
    if (m_debug_enable_display && (!m_debug_display_is_initialized)) {
      displayer = new vpDisplayX(m_I);
      // displayer.init(m_I);
      vpDisplay::setTitle(m_I, "Megapose object pose estimation");
      m_debug_display_is_initialized = true;
    }
    if (m_debug_enable_display && m_debug_display_is_initialized) {
      vpDisplay::display(m_I);
    }
  }


//! [Detect]
/*
 * Run the detection network on an image in order to find a specific object.
 * The best matching detection is returned:
 * - If a previous Megapose estimation is available, find the closest match in the image (Euclidean distance between centers)
 * - Otherwise, take the detection with highest confidence
 * If no detection corresponding to detectionLabel is found, then std::nullopt is returned
 */
  std::optional<vpRect> detectObjectForInitMegaposeDnn(vpDetectorDNNOpenCV &detector, const cv::Mat &I,
    const std::string &detectionLabel)
  {
    std::vector<vpDetectorDNNOpenCV::DetectedFeatures2D> detections_vec;

    detector.detect(I, detections_vec);
    std::vector<vpDetectorDNNOpenCV::DetectedFeatures2D> matchingDetections;
    for (const auto &detection : detections_vec) {
      std::optional<std::string> classnameOpt = detection.getClassName();
      if (classnameOpt) {

        if (*classnameOpt == detectionLabel) {
          matchingDetections.push_back(detection);
        }
      }
    }

    if (matchingDetections.size() == 0) {
      return std::nullopt;
    }
    else if (matchingDetections.size() == 1) {
      if (m_debug_enable_display) {
        // Debug::Log("<Sombra> [detectObjectForInitMegaposeDnn] Single detection found", Color::Green);
        // Debug::Log("<Sombra> [detectObjectForInitMegaposeDnn] Bounding box: ", Color::Green);
        // Debug::Log("<Sombra> [detectObjectForInitMegaposeDnn] Top: " + std::to_string(matchingDetections[0].getBoundingBox().getTop()), Color::Green);
        // Debug::Log("<Sombra> [detectObjectForInitMegaposeDnn] Bottom: " + std::to_string(matchingDetections[0].getBoundingBox().getBottom()), Color::Green);
        // Debug::Log("<Sombra> [detectObjectForInitMegaposeDnn] Left: " + std::to_string(matchingDetections[0].getBoundingBox().getLeft()), Color::Green);
        // Debug::Log("<Sombra> [detectObjectForInitMegaposeDnn] Right: " + std::to_string(matchingDetections[0].getBoundingBox().getRight()), Color::Green);
      }
      return matchingDetections[0].getBoundingBox();
    }
    else {
      // Get detection that is closest to previous object bounding box estimated by Megapose
      // if (previousEstimate) {
      //   vpRect best;
      //   double bestDist = 10000.f;
      //   const vpImagePoint previousCenter = (*previousEstimate).boundingBox.getCenter();
      //   for (const auto &detection : matchingDetections) {
      //     const vpRect detectionBB = detection.getBoundingBox();
      //     const vpImagePoint center = detectionBB.getCenter();
      //     const double matchDist = vpImagePoint::distance(center, previousCenter);
      //     if (matchDist < bestDist) {
      //       bestDist = matchDist;
      //       best = detectionBB;
      //     }
      //   }
      //   return best;

      // }
      // else { // Get detection with highest confidence
      vpRect best;
      double highestConf = 0.0;
      for (const auto &detection : matchingDetections) {
        const double conf = detection.getConfidenceScore();
        if (conf > highestConf) {
          highestConf = conf;
          best = detection.getBoundingBox();
        }
      }
      return best;
    // }
    }
    return std::nullopt;
  }
  void Gusto_CameraParameters_Init(double cam_px, double cam_py, double cam_u0, double cam_v0)
  {
    m_cam.initPersProjWithoutDistortion(cam_px, cam_py, cam_u0, cam_v0);
  }


  void Gusto_MegaPoseServer_Init()
  {
    Debug::Log(megapose_initialized, Color::Red);

    try {
      megapose = std::make_shared<vpMegaPose>(megaposeAddress, megaposePort, cam, height, width);
      megapose_initialized = true;
    }
    catch (...) {
      throw vpException(vpException::ioError, "Could not connect to Megapose server at " + megaposeAddress + " on port " + std::to_string(megaposePort));
    }

    megaposeTracker = new vpMegaPoseTracker(megapose, objectName, refinerIterations);
    megaposeTracker_initialized = true;

    megapose->setCoarseNumSamples(coarseNumSamples);
    const std::vector<std::string> allObjects = megapose->getObjectNames();

    if (std::find(allObjects.begin(), allObjects.end(), objectName) == allObjects.end()) {
      throw vpException(vpException::badValue, "Object " + objectName + " is not known by the Megapose server!");
    }

  }


  void Gusto_Init(const char *config_path)
  {

    Debug::Log(std::filesystem::current_path().generic_string(), Color::Red);
    int argc = 3;
    const char *argv[3] = { "C# to C++ binding config", "--config", config_path };

    vpJsonArgumentParser parser("Single object tracking with Megapose", "--config", "/");
    parser.addArgument("width", width, true, "The image width")
      .addArgument("height", height, true, "The image height")
      .addArgument("camera", cam, true, "The camera intrinsic parameters. Should correspond to a perspective projection model without distortion.")
      .addArgument("video-device", videoDevice, true, "Video device")
      .addArgument("object", objectName, true, "Name of the object to track with megapose.")
      // .addArgument("detectionMethod", detectionMethod, true, "How to perform detection of the object to get the bounding box:"
      //   " \"click\" for user labelling, \"dnn\" for dnn detection.")
      .addArgument("reinitThreshold", reinitThreshold, false, "If the Megapose score falls below this threshold, then a reinitialization is be required."
        " Should be between 0 and 1")
      .addArgument("megapose/address", megaposeAddress, true, "IP address of the Megapose server.")
      .addArgument("megapose/port", megaposePort, true, "Port on which the Megapose server listens for connections.")
      .addArgument("megapose/refinerIterations", refinerIterations, false, "Number of Megapose refiner model iterations."
        "A higher count may lead to better accuracy, at the cost of more processing time")
      .addArgument("megapose/initialisationNumSamples", coarseNumSamples, false, "Number of Megapose renderings used for the initial pose estimation.")

      .addArgument("detector/model-path", detectorModelPath, true, "Path to the model")
      .addArgument("detector/config", detectorConfig, true, "Path to the model configuration. Set to none if config is not required.")
      .addArgument("detector/framework", detectorFramework, true, "Detector framework")
      .addArgument("detector/type", detectorTypeString, true, "Detector type")
      .addArgument("detector/labels", labels, true, "Detection class labels")
      .addArgument("detector/mean/red", detectorMeanR, false, "Detector mean red component. Used to normalize image")
      .addArgument("detector/mean/green", detectorMeanG, false, "Detector mean green component. Used to normalize image")
      .addArgument("detector/mean/blue", detectorMeanB, false, "Detector mean red component. Used to normalize image")
      .addArgument("detector/confidenceThreshold", detectorConfidenceThreshold, false, "Detector confidence threshold. "
        "When a detection with a confidence below this threshold, it is ignored")
      .addArgument("detector/nmsThreshold", detectorNmsThreshold, false, "Detector non maximal suppression threshold.")
      .addArgument("detector/filterThreshold", detectorFilterThreshold, false)
      .addArgument("detector/scaleFactor", detectorScaleFactor, false, "Pixel intensity rescaling factor. If set to 1/255, then pixel values are between 0 and 1.")
      .addArgument("detector/swapRedAndBlue", detectorSwapRB, false, "Whether to swap red and blue channels before feeding the image to the detector.");

    parser.parse(argc, argv);


    vpDetectorDNNOpenCV::DNNResultsParsingType detectorType = vpDetectorDNNOpenCV::dnnResultsParsingTypeFromString(detectorTypeString);
    vpDetectorDNNOpenCV::NetConfig netConfig(detectorConfidenceThreshold, detectorNmsThreshold, labels, cv::Size(width, height), detectorFilterThreshold);

    dnn = vpDetectorDNNOpenCV(netConfig, detectorType);

    Debug::Log("Tring to detect object: " + objectName, Color::Blue);
    // if (detectionMethod == DetectionMethod::DNN) {
    dnn.readNet(detectorModelPath, detectorConfig, detectorFramework);
    dnn.setMean(detectorMeanR, detectorMeanG, detectorMeanB);
    dnn.setScaleFactor(detectorScaleFactor);
    // dnn.setSwapRB(detectorSwapRB);
    dnn.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    dnn.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
  }
  // }
  bool Gusto_Detection2D_Process(double *bbox_xywh, double *detection_time)
  {

    double t_start = vpTime::measureTimeMs();
    bool has_det = false;

    // Detection

    detection = detectObjectForInitMegaposeDnn(dnn, frame, objectName);
    // vpRect(detection.value().getLeft(), detection.value().getTop(), detection.value().getWidth(), detection.value().getHeight()), vpColor::red, false, 2);
    if (detection) {
      has_det = true;
      bbox_xywh[0] = detection.value().getLeft();
      bbox_xywh[1] = detection.value().getTop();
      bbox_xywh[2] = detection.value().getWidth();
      bbox_xywh[3] = detection.value().getHeight();
    }

    *detection_time = vpTime::measureTimeMs() - t_start;

    if (m_debug_enable_display && m_debug_display_is_initialized) {
      std::stringstream ss;
      ss << "Loop time: " << *detection_time << "   ";
      ss << "Width: " << frame.cols << "   " << "Height: " << frame.rows << "   ";

      vpDisplay::displayText(m_I, 20, 20, ss.str(), vpColor::red);
      vpDisplay::flush(m_I);
    }
    return has_det;
  }



  vpColor interpolate(const vpColor &low, const vpColor &high, const float f)
  {
    const float r = ((float)high.R - (float)low.R) * f;
    const float g = ((float)high.G - (float)low.G) * f;
    const float b = ((float)high.B - (float)low.B) * f;
    return vpColor((unsigned char)r, (unsigned char)g, (unsigned char)b);
  }

  void displayScore(const vpImage<vpRGBa> &I, float score)
  {
    const unsigned top = static_cast<unsigned>(I.getHeight() * 0.85f);
    const unsigned height = static_cast<unsigned>(I.getHeight() * 0.1f);
    const unsigned left = static_cast<unsigned>(I.getWidth() * 0.05f);
    const unsigned width = static_cast<unsigned>(I.getWidth() * 0.5f);
    vpRect full(left, top, width, height);
    vpRect scoreRect(left, top, width * score, height);
    const vpColor low = vpColor::red;
    const vpColor high = vpColor::green;
    const vpColor c = interpolate(low, high, score);
    if (m_debug_enable_display && m_debug_display_is_initialized) {
      vpDisplay::displayRectangle(I, full, c, false, 5);
      vpDisplay::displayRectangle(I, scoreRect, c, true, 1);
    }
  }
  void overlayRender(vpImage<vpRGBa> &I, const vpImage<vpRGBa> &overlay)
  {
    // std::cout << "I size" << I.getHeight() << " " << I.getWidth() << std::endl;
    // std::cout << "overlay size" << overlay.getHeight() << " " << overlay.getWidth() << std::endl;
    // if (I.getHeight() != overlay.getHeight() || I.getWidth() != overlay.getWidth()) {
    //   std::cerr << "overlayRender: I and overlay must be of the same size" << std::endl;
    //   return;
    // }

    const vpRGBa black = vpRGBa(0, 0, 0);
    for (unsigned int i = 0; i < I.getHeight(); ++i) {
      for (unsigned int j = 0; j < I.getWidth(); ++j) {
        if (overlay[i][j] != black) {
          I[i][j] = overlay[i][j];
        }
      }
    }
  }

  bool Gusto_MegaPose_Tracking_Process(float *position, float *rotation)
  {
    bool reinit = false;
    bool overlayModel = false;

    // Debug::Log("Sombra: has_track_future = ", Color::Red);
    // Debug::Log(has_track_future, Color::Red);
    // if (has_track_future) {
    //   Debug::Log("Sombra: trackerFuture Status = ", Color::Red);
    //   Debug::Log(trackerFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready, Color::Red);
    // }

    if (!callMegapose && trackerFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {

      megaposeEstimate = trackerFuture.get();

      if (tracking) {
        megaposeTimes.push_back(vpTime::measureTimeMs() - megaposeStartTime);
      }
      callMegapose = true;
      tracking = true;
      // if (overlayModel) {
      //   overlayImage = megapose->viewObjects({ objectName }, { megaposeEstimate.cTo }, "full");
      // }

      if (megaposeEstimate.score < reinitThreshold) { // If confidence is low, require a reinitialisation with 2D detection
        initialized = false;
        detection = std::nullopt;
        reinit = true;
      }
    }


  //   //! [Check megapose]
  //   //! [Call MegaPose]
    if (callMegapose) {
      if (!initialized) {
        tracking = false;
        if (detection) {
          // Debug::Log("Sombra: <Client> Initialising Megapose with 2D detection", Color::Blue);

          lastDetection = *detection;
          // Debug::Log("Sombra: megaposeTracker->init(m_I, lastDetection)", Color::Red);
            // vpImage<vpRGBa> debug_I;
            // vpRect debug_lastDetection; // Last detection (initialization)
            // trackerFuture = megaposeTracker->init(debug_I, debug_lastDetection);
          trackerFuture = megaposeTracker->init(m_I, lastDetection);
          initialized = true;
          callMegapose = false;

          if (m_debug_enable_display && m_debug_display_is_initialized) {
            vpDisplay::displayRectangle(m_I, vpRect(detection.value().getLeft(), detection.value().getTop(), detection.value().getWidth(), detection.value().getHeight()), vpColor::red, false, 2);
            vpDisplay::flush(m_I);
          }
        }
      }
      else {
        trackerFuture = megaposeTracker->track(m_I);
        callMegapose = false;
        megaposeStartTime = vpTime::measureTimeMs();
      }
    }

    if (tracking) {
      if (m_debug_enable_display && m_debug_display_is_initialized) {
        if (overlayModel) {
          overlayRender(m_I, overlayImage);
          vpDisplay::display(m_I);
        }
        vpDisplay::displayFrame(m_I, megaposeEstimate.cTo, cam, 0.05, vpColor::none, 3);
      }
      // memcpy(position, megaposeEstimate.cTo.getTranslationVector().data, 3 * sizeof(float));
      // memcpy(rotation, megaposeEstimate.cTo.getRotationMatrix().data, 9 * sizeof(float));
      for (size_t i = 0; i < 3; i++) {
        position[i] = megaposeEstimate.cTo.getTranslationVector().data[i];
        for (size_t j = 0; j < 3; j++) {
          rotation[i * 3 + j] = megaposeEstimate.cTo.getRotationMatrix().data[i * 3 + j];
        }
      }

      displayScore(m_I, megaposeEstimate.score);
    }
    if (m_debug_enable_display && m_debug_display_is_initialized) {
      vpDisplay::flush(m_I);
    }
    return reinit;
  }
}
