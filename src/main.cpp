#include <Arduino.h>
#include <Wire.h>
#include <MimirTesting.h>

MimirTesting mimirTesting; /* Time ESP32 will go to sleep (in seconds) */

void setup()
{
  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER)
  {

    Wire.begin();
    mimirTesting.initDisplay(115200);
    mimirTesting.initNeoPixels(50);
    mimirTesting.initSensors();
    mimirTesting.initWIFI();
  }
  mimirTesting.initTimer();
  mimirTesting.initDash();
  mimirTesting.readBattery();
  mimirTesting.readSensors(true);
  mimirTesting.WiFi_ON();
  mimirTesting.sendData(true);
  mimirTesting.WiFi_OFF();
  mimirTesting.SLEEP();
}

void loop()
{
}