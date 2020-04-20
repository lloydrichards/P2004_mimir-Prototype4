#include <Arduino.h>
#include <Wire.h>
#include <MimirTesting.h>

MimirTesting mimirTesting; /* Time ESP32 will go to sleep (in seconds) */

void setup()
{
  pinMode(39, INPUT);
  mimirTesting.initTime();
  mimirTesting.initDisplay(115200);
  Serial.println(digitalRead(39));
  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER)
  {
    Serial.println("Initializing...");
    if (digitalRead(39) == LOW)
    {
      Serial.println("Button Pressed...");
      mimirTesting.forceStartWiFi();
    }
    Wire.begin();

    mimirTesting.initNeoPixels(50);
    mimirTesting.initConfig();
    mimirTesting.initSensors();
    mimirTesting.initWIFI();
  }
  if (mimirTesting.timeToSync())
  {
    mimirTesting.dailySync();
  }
  mimirTesting.DisplayDeviceInfo();

  //mimirTesting.initDash();
  //mimirTesting.readBattery();
  mimirTesting.readSensors(true);
  mimirTesting.WiFi_ON();
  mimirTesting.sendData(true);
  mimirTesting.WiFi_OFF();
  //mimirTesting.forceStartWiFi();
  mimirTesting.SLEEP();
}

void loop()
{

  delay(5000);
}