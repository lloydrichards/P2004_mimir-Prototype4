//Version 1.0 of the Mimir component testing Library
//Model: Prototype 3
//Date:  March 14th 2020
//Author: Lloyd

#ifndef MimirTesting_h
#define MimirTesting_h

#include "Arduino.h"
#include <Wire.h>
#include <SD.h>
#include <WiFiManager.h>

class MimirTesting
{
public:
  MimirTesting();
  void initDisplay(int baudRate = 115200);
  void initNeoPixels(int brightness = 50);
  void initSensors();
  void initWIFI();
  void forceStartWiFi();
  void i2cScanner();
  void testNeoPixels(int repeat = 3, int delay = 500);
  void testBattery();
  void testHTTPRequest();
  bool beginBH1715(uint8_t addr, TwoWire &wirePort = Wire);
  float readBH1715(int addr);
  void readSensors();
  void sendData();
  void printSensors();
  void WiFi_ON();
  void WiFi_OFF();
  void SLEEP();
  void readBattery();

private:
  int _BATTERY = 0;
  int _SENSOR = 0;
  int _WIFI = 0;
  int _SERVER = 0;
  int _MICROSD = 0;

  String _IP_ADDRESS;
  String _USER = "Lloyd Richards";
  String _USER_ID = "P2003_PROTOTYPE";
  String _DEVICE_ID = "PROTOTYPE3_03";

  float temp1;
  float temp2;
  float temp3;
  float alt;
  float hum1;
  float hum2;
  float pres;
  float lux1;
  float lux2;
  float lux3;
  float lux4;
  float avgLux;
  float eCO2;
  float tVOC;

  void writeFile(fs::FS &fs, const char *path, const char *message);
  void appendFile(fs::FS &fs, const char *path, const char *message);
  void printValue(float value, const char *type, const char *unit, int decimel = 2);
  void getIPAddress();
  void blinkPixel(int pixel, int R = 255, int G = 0, int B = 0, int repeat = 1);
  float getBatteryVoltage();
  String packageJSON();

  static uint32_t Colour(uint8_t r, uint8_t g, uint8_t b)
  {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
  }
};
#endif