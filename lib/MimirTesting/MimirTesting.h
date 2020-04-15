//Version 1.0 of the Mimir component testing Library
//Model: Prototype 3
//Date:  March 14th 2020
//Author: Lloyd

#ifndef MimirTesting_h
#define MimirTesting_h

#include "Arduino.h"
#include <Wire.h>
#include <SD.h>
#include "time.h"
#include <esp_now.h>
#include <WiFi.h>
#include <WiFiManager.h>

enum alignment
{
  LEFT,
  RIGHT,
  CENTER
};


class MimirTesting
{
public:
  MimirTesting();

  void initDisplay(int baudRate = 115200);
  void initNeoPixels(int brightness = 50);
  void initSensors(bool display = false);
  void initWIFI(bool display = false);
  void initDash();
  void initTimer();
  void initConfig();
  void initESPNOW();

  void i2cScanner();
  void testNeoPixels(int repeat = 3, int delay = 500);

  void readSensors(bool display = false);
  void readBattery(bool display = false);

  void DisplayDeviceInfo();

  void sendDataWIFI(bool display = false);
  void sendDataESPNOW();
  void startESPNOW();
  void WiFi_ON();
  void WiFi_OFF();
  void forceStartWiFi();
  void testHTTPRequest();

  void SLEEP();

private:
  uint8_t allBroadcastAddress[4][6] = {
      {0x24, 0x6F, 0x28, 0xB1, 0xDC, 0xE4},  //Device #1
      {0x24, 0x6F, 0x28, 0xB1, 0xD4, 0x58},  //Device #2
      {0x24, 0x6F, 0x28, 0xB1, 0xD5, 0x30},  //Device #3
      {0x24, 0x6F, 0x28, 0xB1, 0xDC, 0xD8}}; //Device #4

  uint8_t broadcastAddress[6] = {0x24, 0x6F, 0x28, 0xB1, 0xDC, 0xE4};
  esp_now_peer_info_t peerInfo;

  int _BATTERY = 0;
  int _SENSOR = 0;
  int _WIFI = 0;
  int _SERVER = 0;
  int _MICROSD = 0;

  String _IP_ADDRESS;
  String _MAC_ADDRESS;
  char _USER[40];
  char _USER_ID[40];
  char _DEVICE_ID[40];

  String TimeStr, DateStr, ErrorMessage; // strings to hold time and date
  const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

  int StartTime = 0, CurrentHour = 0, CurrentMin = 0, CurrentSec = 0;
  long SleepDuration = 15;

  int wifi_signal;
  int batteryPercent;

  float temp1;
  float temp2;
  float temp3;
  float alt;
  float hum1;
  float hum2;
  float pres;
  float lux1;
  float lux2;
  float eCO2;
  float tVOC;

  bool SHT31D_L_STATUS = false;
  bool SHT31D_H_STATUS = false;
  bool VEML6030_STATUS = false;
  bool TEMT600_STATUS = false;
  bool CCS811B_STATUS = false;
  bool BMP280_STATUS = false;

  void writeFile(fs::FS &fs, const char *path, const char *message);
  void appendFile(fs::FS &fs, const char *path, const char *message);
  void saveConfig();

  void printValue(float value, const char *type, const char *unit, int decimel = 2);
  void getIPAddress();

  void DisplayWiFiIcon(int x, int y);
  void DisplayBatteryIcon(int x, int y);
  void DisplaySensors();
  void DisplayReadings();
  void DisplayWiFiSetup();
  void DisplayWiFiCredentials();
  void DisplaySentData(int httpResponseCode, String response);

  void drawString(int x, int y, String text, alignment align);
  void blinkPixel(int pixel, int R = 255, int G = 0, int B = 0, int repeat = 1);

  bool SetupTime();
  bool UpdateLocalTime();
  float getBatteryVoltage();

  static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
  static void onDataReceived(const uint8_t *mac_addr, const uint8_t *incomingData, int len);
  String packageJSON();

  String mac2String(byte ar[]);

  static uint32_t Colour(uint8_t r, uint8_t g, uint8_t b)
  {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
  }
};
#endif