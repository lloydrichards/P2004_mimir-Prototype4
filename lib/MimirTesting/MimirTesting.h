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

#include <NeoPixelBrightnessBus.h>

enum alignment
{
  LEFT,
  RIGHT,
  CENTER
};

enum STATUS_LED
{
  BATTERY_LED,
  MICROSD_LED,
  SENSOR_LED,
  SERVER_LED,
  WIFI_LED
};

enum STATUS_ERROR
{
  ERROR_READ = -3,
  ERROR_WRITE = -2,
  ERROR_UNDEFINED = -1,
  UNMOUNTED = 0,
  SUCCESS = 1
};

enum STATUS_BATTERY
{
  CRITICAL_BATTERY,
  LOW_BATTERY,
  GOOD_BATTERY,
  FULL_BATTERY,
  CHARGING

};

class MimirTesting
{
public:
  MimirTesting();

  void initDisplay(int baudRate = 115200);
  void initNeoPixels(int brightness = 50);
  void initSensors(bool display = false);
  void initWIFI(bool display = false);
  void initMicroSD(bool display = false);
  void initDash();
  void initTimer();
  void initConfig();

  void i2cScanner();
  void testNeoPixels(int repeat = 3, int delay = 500);
  void busyNeoPixels();
  void statusNeoPixels(int delay = 100);
  void activeNeoPixels(STATUS_LED system, uint32_t colour, int repeat);

  void readSensors(bool display = false);
  void readBattery(bool display = false);

  void DisplayDeviceInfo();

  void sendData(bool display = false);
  void logData(bool display = false);
  void WiFi_ON();
  void WiFi_OFF();
  void forceStartWiFi();
  void testHTTPRequest();

  void SLEEP();
  void WAKEUP_REASON();

private:
  enum STATUS_BATTERY BATTERY_STATUS;
  enum STATUS_ERROR SENSOR_STATUS = UNMOUNTED;
  enum STATUS_ERROR WIFI_STATUS = UNMOUNTED;
  enum STATUS_ERROR SERVER_STATUS = UNMOUNTED;
  enum STATUS_ERROR MICROSD_STATUS = UNMOUNTED;

  String _IP_ADDRESS;
  char _USER[40];
  char _USER_ID[40];
  char _DEVICE_ID[40];

  char filename[16] = "/0000-00-00.txt";

  String TimeStr, DateStr, ErrorMessage; // strings to hold time and date
  const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

  int StartTime = 0, CurrentHour = 0, CurrentMin = 0, CurrentSec = 0;
  long SleepDuration = 15; //minutes of the hour. eg 15 would wake up at XX:00, XX:15, XX:30 and XX:45

  int wifi_signal;
  int batteryPercent;

  float temp1;
  float temp2;
  float temp3;
  float alt;
  float hum1;
  float hum2;
  float pres;
  float lux;
  float uvA;
  float uvB;
  float uvIndex;
  float eCO2;
  float tVOC;
  int16_t compassX;
  int16_t compassY;
  int16_t compassZ;
  float bearing;

  enum STATUS_ERROR SHT31D_L_STATUS = UNMOUNTED;
  enum STATUS_ERROR SHT31D_H_STATUS = UNMOUNTED;
  enum STATUS_ERROR VEML6030_STATUS = UNMOUNTED;
  enum STATUS_ERROR VEML6075_STATUS = UNMOUNTED;
  enum STATUS_ERROR CCS811_STATUS = UNMOUNTED;
  enum STATUS_ERROR BMP280_STATUS = UNMOUNTED;
  enum STATUS_ERROR COMPASS_STATUS = UNMOUNTED;

  void writeFile(fs::FS &fs, const char *path, const char *message);
  void appendFile(fs::FS &fs, const char *path, const char *message);
  void saveConfig();
  void createFileName(char date[]);

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
  RgbColor getStatusColor(enum STATUS_ERROR STATUS);
  RgbColor getBatteryColor(enum STATUS_BATTERY STATUS);

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