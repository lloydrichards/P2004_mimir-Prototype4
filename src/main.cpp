#include <Arduino.h>
#include <MimirTesting.h>

MimirTesting mimirTesting; /* Time ESP32 will go to sleep (in seconds) */

void setup()
{
  mimirTesting.initTimer();
  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER)
  {
    Serial.begin(115200);
    Wire.begin();
    mimirTesting.initDisplay(115200);
    mimirTesting.initNeoPixels(35);
    //mimirTesting.i2cScanner();
    mimirTesting.initConfig();
    mimirTesting.initMicroSD();
    mimirTesting.initSensors();
    mimirTesting.initWIFI();
  }
  //mimirTesting.initDash();
  mimirTesting.readBattery();
  mimirTesting.readSensors(true);
  mimirTesting.WiFi_ON();
  mimirTesting.sendData(true);
  mimirTesting.WiFi_OFF();
  mimirTesting.logData();
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