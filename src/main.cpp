#include <Arduino.h>
#include <Wire.h>
#include <MimirTesting.h>

MimirTesting mimirTesting;

void setup()
{

  Wire.begin();
  mimirTesting.initDisplay(115200);
  mimirTesting.initNeoPixels(50);
  mimirTesting.initSensors();
  mimirTesting.initWIFI();
  mimirTesting.i2cScanner();
}

void loop()
{
  mimirTesting.currentStatus();
  mimirTesting.readSensors();
  mimirTesting.sendData();
  mimirTesting.printSensors();
  delay(60*1000);
}