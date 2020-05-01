#include <Arduino.h>
#include <MimirTesting.h>

MimirTesting mimirTesting; /* Time ESP32 will go to sleep (in seconds) */

void setup()
{
  mimirTesting.initTimer();
  mimirTesting.WAKEUP_REASON();
  Serial.begin(115200);
  mimirTesting.initDisplay(115200);
  mimirTesting.initNeoPixels();
  //mimirTesting.i2cScanner();
  mimirTesting.initConfig();
  mimirTesting.initMicroSD();
  mimirTesting.initSensors();
  mimirTesting.initWIFI();
  //mimirTesting.initDash();
  mimirTesting.readBattery();
  mimirTesting.readSensors(true);
  mimirTesting.WiFi_ON();
  mimirTesting.sendData(true);
  mimirTesting.WiFi_OFF();
  mimirTesting.logData();
  mimirTesting.readBattery();
  mimirTesting.readSensors(true);
  mimirTesting.statusNeoPixels();
  //mimirTesting.forceStartWiFi();
  mimirTesting.SLEEP();
}

void loop()
{
  // mimirTesting.DisplayDeviceInfo();
  //mimirTesting.readSensors(true);
  //mimirTesting.logData();
  //delay(5000);
  //mimirTesting.readBattery(true);
  //delay(5000);
}