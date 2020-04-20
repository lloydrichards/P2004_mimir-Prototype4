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
#include <WiFiManager.h>

//Stored Variables in RTC

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

  int BATTERY_PERCENT;

  void initDisplay(int baudRate = 115200);
  void initNeoPixels(int brightness = 50);
  void initSensors(bool display = false);
  void initWIFI(bool display = false);
  void initDash();
  void initTime();
  void initConfig();

  void i2cScanner();
  void testNeoPixels(int repeat = 3, int delay = 500);

  void readSensors(bool display = false);
  void readBattery(bool display = false);

  void DisplayDeviceInfo();

  void sendData(bool display = false);
  void WiFi_ON();
  void WiFi_OFF();
  void forceStartWiFi();
  void testHTTPRequest();

  bool timeToSync();
  bool dailySync();
  bool dailySyncServer();

  void SLEEP();

private:
  //RTC_DATA_ATTR bool _ALREADY_SYNCED = false;
  //RTC_DATA_ATTR bool _SYNC_NOW = false;
  int _BATTERY = 0;
  int _SENSOR = 0;
  int _WIFI = 0;
  int _SERVER = 0;
  int _MICROSD = 0;

  String _IP_ADDRESS;
  char _USER[40];
  char _USER_ID[40];
  char _DEVICE_ID[40];

  String TimeStr, DateStr, ErrorMessage; // strings to hold time and date
  const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

  int StartTime = 0;
  //RTC_DATA_ATTR int CurrentHour = 0, CurrentMin = 0, CurrentSec = 0;
  long SleepDuration = 5; //Will wake up on a rounded incriment of this time in minute (eg. 15 = X:00, X:15, X:30x X:45)

  int wifi_signal;

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
  String packageJSON();

  static uint32_t Colour(uint8_t r, uint8_t g, uint8_t b)
  {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
  }
};
#endif