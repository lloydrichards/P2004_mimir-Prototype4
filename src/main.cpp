#include <Arduino.h>
#include <MimirTesting.h>

MimirTesting mimirTesting; /* Time ESP32 will go to sleep (in seconds) */

void setup()
{
  mimirTesting.initTimer();
  mimirTesting.WAKEUP_REASON();


  Serial.begin(115200);
  mimirTesting.initDisplay(115200);
  mimirTesting.initNeoPixels(true);
  //mimirTesting.i2cScanner();
  mimirTesting.initConfig();
  mimirTesting.readBattery(false, true);
  mimirTesting.initMicroSD(false, true);
  mimirTesting.initSensors(false, true);
  mimirTesting.initWIFI(false, true);
  //mimirTesting.initDash();
   
   mimirTesting.readSensors(false, true);
  mimirTesting.WiFi_ON();
  mimirTesting.sendData(true, true);
  mimirTesting.WiFi_OFF();
 mimirTesting.logData(false, true);
 mimirTesting.DisplayDeviceInfo();
  //mimirTesting.statusNeoPixels(100,true);
  //mimirTesting.forceStartWiFi();
 // mimirTesting.SLEEP();
}

void loop()
{
  // mimirTesting.DisplayDeviceInfo();
  mimirTesting.readSensors(false, true);
  //mimirTesting.logData();
  //delay(5000);
  mimirTesting.initDash();
  delay(60000);
}