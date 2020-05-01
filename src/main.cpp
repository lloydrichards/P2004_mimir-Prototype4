#include <Arduino.h>
#include <MimirTesting.h>

MimirTesting mimirTesting; /* Time ESP32 will go to sleep (in seconds) */

void setup()
{
  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER)
  {
    Serial.begin(115200);
    Wire.begin();
    mimirTesting.initDisplay(115200);
    mimirTesting.initNeoPixels(50);
    mimirTesting.i2cScanner();
    mimirTesting.initConfig();
    mimirTesting.initSensors();
    mimirTesting.initWIFI();
  }
  mimirTesting.initTimer();
  //mimirTesting.initDash();
  mimirTesting.readBattery();
  mimirTesting.readSensors(true);
  // mimirTesting.WiFi_ON();
  // mimirTesting.sendData(true);
  // mimirTesting.WiFi_OFF();
  //mimirTesting.forceStartWiFi();
}

void loop()
{
 // mimirTesting.DisplayDeviceInfo();
  mimirTesting.readSensors(true);
  delay(5000);
  //mimirTesting.readBattery(true);
  //delay(5000);
}