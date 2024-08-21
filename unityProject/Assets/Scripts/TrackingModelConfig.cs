using System;
using System.Collections;
using System.Collections.Generic;

public class TrackingModelConfig
{
    public int Height { get; set; }
    public int Width { get; set; }
    public string VideoDevice { get; set; }
    public string Object { get; set; }
    public string DetectionMethod { get; set; }
    public double ReinitThreshold { get; set; }
    public Camera Camera { get; set; }
    public Detector Detector { get; set; }
    public Megapose Megapose { get; set; }
    public Dictionary<string, string> MegaposeObjectToDetectorClass { get; set; }
}

public class Camera
{
    public string Model { get; set; }
    public double Px { get; set; }
    public double Py { get; set; }
    public double U0 { get; set; }
    public double V0 { get; set; }
}

public class Detector
{
    public string ModelPath { get; set; }
    public string Config { get; set; }
    public string Type { get; set; }
    public string Framework { get; set; }
    public List<string> Labels { get; set; }
    public Mean Mean { get; set; }
    public double ConfidenceThreshold { get; set; }
    public double NmsThreshold { get; set; }
    public double FilterThreshold { get; set; }
    public double ScaleFactor { get; set; }
    public bool SwapRedAndBlue { get; set; }
}

public class Mean
{
    public double Red { get; set; }
    public double Blue { get; set; }
    public double Green { get; set; }
}

public class Megapose
{
    public string Address { get; set; }
    public int Port { get; set; }
    public int RefinerIterations { get; set; }
    public int InitialisationNumSamples { get; set; }
}
