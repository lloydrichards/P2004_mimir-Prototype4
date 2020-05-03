#include <Arduino.h>
#include <MimirTesting.h>
#include "esp_sleep.h"
#include "sdkconfig.h"

#define GPIO_INPUT_IO_TRIGGER 0
#define GPIO_DEEP_SLEEP_DURATION 60
#define Threshold 20

RTC_DATA_ATTR int count;
touch_pad_t touchPin;

MimirTesting mimirTesting;

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_UNDEFINED:
    Serial.println("In case of deep sleep, reset was not caused by exit from deep sleep");
    break;
  case ESP_SLEEP_WAKEUP_ALL:
    Serial.println("Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source");
    break;
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_UART:
    Serial.println("Wakeup caused by UART (light sleep only)");
    break;
  default:
    Serial.println("Wakeup was not caused by deep sleep");
    break;
  }
}

//Function that prints the touchpad by which ESP32 has been awaken from sleep
void print_wakeup_touchpad()
{
  touch_pad_t pin;
  touchPin = esp_sleep_get_touchpad_wakeup_status();
  switch (touchPin)
  {
  case 0:
    Serial.println("Touch detected on GPIO 4");
    break;
  case 1:
    Serial.println("Touch detected on GPIO 0");
    break;
  case 2:
    Serial.println("Touch detected on GPIO 2");
    break;
  case 3:
    Serial.println("Touch detected on GPIO 15");
    break;
  case 4:
    Serial.println("Touch detected on GPIO 13");
    break;
  case 5:
    Serial.println("Touch detected on GPIO 12");
    break;
  case 6:
    Serial.println("Touch detected on GPIO 14");
    break;
  case 7:
    Serial.println("Touch detected on GPIO 27");
    break;
  case 8:
    Serial.println("Touch detected on GPIO 33");
    break;
  case 9:
    Serial.println("Touch detected on GPIO 32");
    break;
  default:
    Serial.println("Wakeup not by touchpad");
    break;
  }
}

void callback()
{
  Serial.println("Touch Sensor Pressed!");
  mimirTesting.statusNeoPixels(100, true);
}

void setup()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  Serial.begin(115200);
  mimirTesting.initTimer();
  mimirTesting.initDisplay(115200);

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_UNDEFINED:
    //When RESET is Pressed
    mimirTesting.initNeoPixels(true);
    mimirTesting.i2cScanner();
    mimirTesting.initConfig();
    mimirTesting.readBattery(false, true);
    mimirTesting.initMicroSD(false, true);
    mimirTesting.initSensors(true, true);
    mimirTesting.initWIFI(true, true);
    mimirTesting.DisplayDeviceInfo();
    break;
  case ESP_SLEEP_WAKEUP_EXT0:
    //When GPIO 39 is Pressed
    mimirTesting.statusNeoPixels(100, true);
    Serial.println("GPIO 39 was pressed!");
    mimirTesting.forceStartWiFi();
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    //When TIMER goes off
    mimirTesting.initConfig();
    mimirTesting.readBattery(false, false);
    mimirTesting.initMicroSD(false, false);
    mimirTesting.resetSensors(false, false);
    //mimirTesting.initWIFI(false, false);
    mimirTesting.readSensors(false, false);
    mimirTesting.WiFi_ON();
    mimirTesting.sendData(true, false);
    mimirTesting.WiFi_OFF();
    mimirTesting.logData(false, false);
    mimirTesting.statusNeoPixels(100, true);
    break;
  default:
    break;
  }
  //mimirTesting.WAKEUP_REASON();

  
  mimirTesting.initDash();

  //CONFIG Deep Sleep Timer
  // Serial.println("Config Sleep Timer");
  // esp_sleep_enable_timer_wakeup(1000000LL * GPIO_DEEP_SLEEP_DURATION); // set timer but don't sleep now

  //CONFIG Deep Sleep Touch
  // Serial.println("Config Sleep Touch");
  // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO);
  // esp_sleep_enable_touchpad_wakeup();

  //CONFIG Deep Sleep GPIO Pin
  // Serial.println("Config Sleep Pin");
  // gpio_pullup_en(GPIO_NUM_39);    // use pullup on GPIO
  // gpio_pulldown_dis(GPIO_NUM_39); // not use pulldown on GPIO

  // esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0); // Wake if GPIO is low

  count++;
  Serial.print("Count: ");
  Serial.println(count);
  // Serial.println("Going to Sleep...");
  // esp_deep_sleep_start();
  //mimirTesting.statusNeoPixels(100,true);
  //mimirTesting.forceStartWiFi();
  mimirTesting.SLEEP();
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