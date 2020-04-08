#include <Arduino.h>
#include <Wire.h>
#include <MimirTesting.h>

MimirTesting mimirTesting;

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 900       /* Time ESP32 will go to sleep (in seconds) */

void setup()
{
  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER)
  {

    Wire.begin();
    mimirTesting.initDisplay(115200);
    //mimirTesting.initNeoPixels(50);
    mimirTesting.initSensors();
   // mimirTesting.initWIFI();
  }
  // mimirTesting.readSensors();
  // mimirTesting.WiFi_ON();
  // mimirTesting.sendData();
  // mimirTesting.WiFi_OFF();
  // mimirTesting.printSensors();
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //mimirTesting.SLEEP();
  //esp_deep_sleep_start();
}

void loop()
{
  mimirTesting.readBattery();
  delay(5000);
}