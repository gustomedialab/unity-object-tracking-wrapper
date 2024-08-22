/****************************************************************************
 *
 * ViSP, open source Visual Servoing Platform software.
 * Copyright (C) 2005 - 2020 by Inria. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact Inria about acquiring a ViSP Professional
 * Edition License.
 *
 * See http://visp.inria.fr for more information.
 *
 * This software was developed at:
 * Inria Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 *
 * If you have questions regarding the use of this file, please contact
 * Inria at visp@inria.fr
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Description:
 * Unity application that shows how to use ViSPUnity plugin.
 *
 *****************************************************************************/

 /*!
  \example Script_ar.cs
  Unity CSharp script that allows to detect and AprilTag and display a cube in Augmented Reality using ViSPUnity plugin.
 */

using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.UI;
using Newtonsoft.Json;




public class Script_ar : MonoBehaviour
{
  public GameObject effect;

  [DllImport("GustoUnityWrapper", CallingConvention = CallingConvention.Cdecl, EntryPoint = "Gusto_ImageUchar_SetFromColor32Array")]
  public static extern void Gusto_ImageUchar_SetFromColor32Array(Color32[] bitmap, int height, int width);
  [DllImport("GustoUnityWrapper", CallingConvention = CallingConvention.Cdecl, EntryPoint = "Gusto_Init")]
  public static extern void Gusto_Init(string ConfigPath);
  [DllImport("GustoUnityWrapper", CallingConvention = CallingConvention.Cdecl, EntryPoint = "Gusto_MegaPoseServer_Init")]
  public static extern void Gusto_MegaPoseServer_Init();
  [DllImport("GustoUnityWrapper", CallingConvention = CallingConvention.Cdecl, EntryPoint = "Gusto_CameraParameters_Init")]
  public static extern void Gusto_CameraParameters_Init(double cam_px, double cam_py, double cam_u0, double cam_v0);
  [DllImport("GustoUnityWrapper", CallingConvention = CallingConvention.Cdecl, EntryPoint = "Gusto_Detection2D_Process")]
  public static extern bool Gusto_Detection2D_Process(double[] has_det, double[] detection_time);
  [DllImport("GustoUnityWrapper", CallingConvention = CallingConvention.Cdecl, EntryPoint = "Gusto_MegaPose_Tracking_Process")]
  public static extern bool Gusto_MegaPose_Tracking_Process(float[] est_position, float[] est_rotation);

  [DllImport("GustoUnityWrapper", CallingConvention = CallingConvention.Cdecl, EntryPoint = "Gusto_CppWrapper_MemoryFree")]
  public static extern void Gusto_CppWrapper_MemoryFree();

      // Use this for initialization
  void OnEnable()
  {
      RegisterDebugCallback(OnDebugCallback);
  }

  //------------------------------------------------------------------------------------------------
  [DllImport("GustoUnityWrapper", CallingConvention = CallingConvention.Cdecl)]
  static extern void RegisterDebugCallback(debugCallback cb);
  //Create string param callback delegate
  delegate void debugCallback(IntPtr request, int color, int size);
  enum Color { red, green, blue, black, white, yellow, orange };
  // [MonoPInvokeCallback(typeof(debugCallback))]
  static void OnDebugCallback(IntPtr request, int color, int size)
  {
      //Ptr to string
      string debug_string = Marshal.PtrToStringAnsi(request, size);

      //Add Specified Color
      debug_string =
          String.Format("{0}{1}{2}{3}{4}",
          "<color=",
          ((Color)color).ToString(),
          ">",
          debug_string,
          "</color>"
          );

      UnityEngine.Debug.Log(debug_string);
  }
  public enum DebugType {
    Enabled,
    Disabled
  };

  WebCamTexture m_webCamTexture;
  Renderer m_renderer;
  WebCamDevice[] m_devices;
  // Reference for GameObject Cube, that is to be moved on the plane
  GameObject m_cube;
  // Reference for GameObject Cube_pivot, that is to be rotated
  GameObject m_cube_pivot;

  // For storing tag characteristics returned by ViSPunity wrapper
  float[] m_tag_cog = new float[2];
  float[] m_tag_length = new float[6];
  float[] m_tag_cMo = new float[16];
  double[] m_detection_time = new double[1];
  double[] bbox_xywh = new double[4];
  float[] est_position = new float[3];
  float[] est_rotation = new float[9];
  bool reinit = true;
  bool m_wct_resolution_updated = false;
  float m_aspect_ratio;
  // For debug log
  bool m_log_start = true;
  bool m_log_process = true;

  //Quaternion m_baseRotation;

  [Header("Camera Identifier")]
  public int camera_id = 0;

  [Header("Camera Parameters")] //some default values provided
  public double cam_px = 600;
  public double cam_py = 600;
  public double cam_u0 = 320;
  public double cam_v0 = 240;

  [Header("Tag Size in [m]")]
  public double tag_size = 0.053;

  [Header("Tag Detection Settings")]
  public float quad_decimate = 1;
  public int nthreads = 1;

  [Header("Debugging Settings")]
  public Script_ar.DebugType debug_display = DebugType.Disabled;

  [Header("Config name")]
  public string ConfigPath;
  public TrackingModelConfig config;


  void Awake()
  {
    Debug.Log(ConfigPath);
  }
  void Start()
  {
    m_devices = WebCamTexture.devices;

    if (m_devices.Length == 0) {
      throw new Exception("No camera device found");
    }

    int max_id = m_devices.Length - 1;
    if (camera_id > max_id) {
      if (m_devices.Length == 1) {
        throw new Exception("Camera with id " + camera_id + " not found. camera_id value should be 0");
      }
      else {
        throw new Exception("Camera with id " + camera_id + " not found. camera_id value should be between 0 and " + max_id.ToString());
      }
    }

    m_webCamTexture = new WebCamTexture(WebCamTexture.devices[camera_id].name, 640, 480, 30);

    m_renderer = GetComponent<Renderer>();
    m_renderer.material.mainTexture = m_webCamTexture;
    //m_baseRotation = transform.rotation;
    m_webCamTexture.Play(); //Start capturing image using webcam

    m_cube = GameObject.Find("Cube");
    m_cube_pivot = GameObject.Find("Cube_pivot");

    // Visp_EnableDisplayForDebug((debug_display == DebugType.Enabled) ? true : false);
    // // Initialize tag detection
    // Visp_CameraParameters_Init(cam_px, cam_py, cam_u0, cam_v0);
    // Visp_DetectorAprilTag_Init(quad_decimate, nthreads);
    Gusto_CameraParameters_Init(cam_px, cam_py, cam_u0, cam_v0);

    // ConfigPath must be a absolute path
    Gusto_Init(ConfigPath);
    Gusto_MegaPoseServer_Init();
    Debug.Log("[Gusto_Detection2D_Init] Finished");
    // For debugging purposes, prints available devices to the console
    if(m_log_start) {
      for(int i = 0; i < m_devices.Length; i++) {
        Debug.Log("Webcam " + i + " available: " + m_devices[i].name);
      }
      Debug.Log("Device name: " + m_webCamTexture.deviceName);
      Debug.Log("Web Cam Texture Resolution init : " + m_webCamTexture.width + " " + m_webCamTexture.height);
      Debug.Log("Tag detection settings: quad_decimate=" + quad_decimate + " nthreads=" + nthreads);
      Debug.Log("Camera parameters: u0=" + cam_u0 + " v0=" + cam_v0 + " px=" + cam_px + " py=" + cam_py);
      Debug.Log("Tag size [m]: " + tag_size);
    }
  }

  /*
    When more than one camera is connected, create a square button on the top
    left part of the display that allows to change the device.
   */
  void OnGUI()
  {
    if (m_devices.Length > 1) {
      if( GUI.Button( new Rect(0,0,100,100), "Switch\nto next\ncamera" )) {
        camera_id ++;
        int id = (camera_id % m_devices.Length);
        Debug.Log("Camera id: " + id);
        m_webCamTexture.Stop();
        m_webCamTexture.deviceName = m_devices[id].name;
        Debug.Log("Switch to new device name: " + m_webCamTexture.deviceName);
        m_webCamTexture.Play();
      }
    }
  }

  void Update()
  {
    if (m_webCamTexture == null) {
      return;
    }
    // Warmup camera to get an updated web cam texture resolution up to Update
    // On OSX, m_webCamTexture.width and m_webCamTexture.height returns always 16 for width and height.
    // After a couple of seconds, Web Cam Texture size is updated
    if (! m_wct_resolution_updated) {
      if (m_webCamTexture.width > 100 && m_webCamTexture.height > 100) {

        Debug.Log("Web Cam Texture Resolution: " + m_webCamTexture.width + " " + m_webCamTexture.height);

        m_wct_resolution_updated = true;

        // Change height and width of m_webCamTexture plane according to the camera resolution
        m_aspect_ratio = (float) m_webCamTexture.width / m_webCamTexture.height;
        transform.localScale = new Vector3(m_aspect_ratio, 1f, 1f);
      }
      else {
        return;
      }
    }


    if (m_log_start) {
      Debug.Log("Image size: " + m_webCamTexture.width + " x " + m_webCamTexture.height);
      m_log_start = false;
    }

    Gusto_ImageUchar_SetFromColor32Array(m_webCamTexture.GetPixels32(), m_webCamTexture.height, m_webCamTexture.width);

    if (reinit){
      bool has_det = Gusto_Detection2D_Process(bbox_xywh, m_detection_time);
      if (has_det) {
        reinit = false;
      }
    }else{
      Debug.Log("reinit = " + reinit);
      bool[] catch_exception = new bool[1];
      catch_exception[0] = false;
      reinit = Gusto_MegaPose_Tracking_Process(est_position, est_rotation);
    }
    Matrix4x4 cTo = new Matrix4x4(
      new Vector4(est_rotation[0], est_rotation[3], est_rotation[6], 0.0f),
      new Vector4(est_rotation[1], est_rotation[4], est_rotation[7], 0.0f),
      new Vector4(est_rotation[2], est_rotation[5], est_rotation[8], 0.0f),
      new Vector4(est_position[0], est_position[1], est_position[2], 1.0f)
    );


    // Debug.Log(cTo.ToString());
    effect.transform.position = new Vector3(est_position[0], est_position[1], est_position[2]);
    effect.transform.rotation = cTo.rotation;

  }

  void OnApplicationQuit()
  {
    Gusto_CppWrapper_MemoryFree();
    Debug.Log("Application ending after " + Time.time + " seconds");
  }
}
