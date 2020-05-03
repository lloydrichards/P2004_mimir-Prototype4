#include <Arduino.h>
#include <MimirTesting.h>
#include "esp_sleep.h"
#include "sdkconfig.h"

#define GPIO_INPUT_IO_TRIGGER 0
#define GPIO_DEEP_SLEEP_DURATION 60
RTC_DATA_ATTR int count;

MimirTesting mimirTesting;

void setup()
{
  Serial.begin(115200);

  mimirTesting.initTimer();
  mimirTesting.WAKEUP_REASON();

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
  mimirTesting.initDash();

  //CONFIG Deep Sleep Timer
  Serial.println("Config Sleep Timer");
  esp_sleep_enable_timer_wakeup(1000000LL * GPIO_DEEP_SLEEP_DURATION); // set timer but don't sleep now

  //CONFIG Deep Sleep GPIO Pin
  Serial.println("Config Sleep Pin");
  gpio_pullup_en(GPIO_NUM_39);                                            // use pullup on GPIO
  gpio_pulldown_dis(GPIO_NUM_39);                                         // not use pulldown on GPIO

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0); // Wake if GPIO is low

  mimirTesting.statusNeoPixels(100, true);

  count++;
  Serial.print("Count: ");
  Serial.println(count);
  Serial.println("Going to Sleep...");
  esp_deep_sleep_start();
  //mimirTesting.statusNeoPixels(100,true);
  //mimirTesting.forceStartWiFi();
  // mimirTesting.SLEEP();
}

void loop()
{
  // mimirTesting.DisplayDeviceInfo();
  // mimirTesting.readSensors();
  // mimirTesting.logData();
  // delay(5000);
  // mimirTesting.initDash();
  // mimirTesting.statusNeoPixels(100, true);
  // delay(60000);
}