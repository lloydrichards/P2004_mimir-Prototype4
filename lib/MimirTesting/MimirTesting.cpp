#include "MimirTesting.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

//Sensor Libraries
#include "Adafruit_SHT31.h"                         //https://github.com/adafruit/Adafruit_SHT31
#include "SparkFun_VEML6030_Ambient_Light_Sensor.h" //https://github.com/sparkfun/SparkFun_Ambient_Light_Sensor_Arduino_Library
//https://github.com/ControlEverythingCommunity/BH1715/blob/master/Arduino/BH1715.ino
#include "Adafruit_CCS811.h" //https://github.com/adafruit/Adafruit_CCS811
#include "BME280I2C.h"

//I2C Addresses
#define addrSHT31D_L 0x44
#define addrSHT31D_H 0x45
#define addrBH1715_L 0x23
#define addrBH1715_H 0x5C
#define addrVEML6030 0x10
#define addrCCS811B 0x5A
#define addrBME280 0x76

#include <Adafruit_NeoPixel.h>

#include <GxEPD.h>
#include <GxGDEH0213B73/GxGDEH0213B73.h>
#include <Fonts/FreeSans9pt7b.h>

//Set Screen SPI
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/17, /*RST=*/16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/16, /*BUSY=*/4);

#define LED_PIN 25
#define LED_COUNT 5
//Define Sensors
Adafruit_SHT31 sht31_L = Adafruit_SHT31();
Adafruit_SHT31 sht31_H = Adafruit_SHT31();
SparkFun_Ambient_Light veml6030(addrVEML6030);
Adafruit_CCS811 ccs811B;
BME280I2C bme280;

//VEML6030 settings
float gain = .125;
int integTime = 100;

//BME280 settings
#define SEALEVELPRESSURE_HPA (1013.25)

//SD setting
int ID;
String dataMessage;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

MimirTesting::MimirTesting()
{
}

void MimirTesting::initDisplay(int baudRate)
{
    display.init(baudRate);
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setRotation(0);
    display.setCursor(2, 20);
    display.println("I want...");
    display.println();
    display.println("PANCAKES!");
    display.update();
    delay(1000);
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, true);
}

void MimirTesting::initNeoPixels(int brightness)
{
    strip.begin();
    strip.clear(); // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();  // Turn OFF all pixels ASAP
    strip.setBrightness(brightness);
}

void MimirTesting::initSensors()
{
    busyLED(1);
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    !sht31_L.begin(addrSHT31D_L) ? display.println("SHT31_L: X ")
                                 : display.println("SHT31_L: O");

    !sht31_H.begin(addrSHT31D_H) ? display.println("SHT31_H: X")
                                 : display.println("SHT31_H: O");
    if (!veml6030.begin())
    {
        display.println("VEML6030: X");
    }
    else
    {
        veml6030.setGain(gain);
        veml6030.setIntegTime(integTime);
        display.println("VEML6030: O");
    };
    pinMode(33, OUTPUT);
    digitalWrite(33, LOW);
    !ccs811B.begin(addrCCS811B) ? display.println("CCS811B: X")
                                : display.println("CCS811B: O");

    ccs811B.setDriveMode(CCS811_DRIVE_MODE_60SEC);
    float _temp = ccs811B.calculateTemperature();
    ccs811B.setTempOffset(_temp - 25.0);

    !bme280.begin() ? display.println("BME280: X")
                    : display.println("BME280: O");

    pinMode(36, OUTPUT);
    digitalWrite(36, LOW);
    !beginBH1715(addrBH1715_L) ? display.println("BH1715_L: X")
                               : display.println("BH1715_L: O");
    !beginBH1715(addrBH1715_H) ? display.println("BH1715_H: X")
                               : display.println("BH1715_H: O");
    pinMode(32, INPUT);
    burnReading(8);
    !isnan(analogRead(32)) ? display.println("TEMT600: O")
                           : display.println("TEMT600: X");

    display.update();
}

void MimirTesting::initWIFI()
{
    busyLED(2);
    WiFiManager wifiManager;
    wifiManager.autoConnect("mimirAP");
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    if (WiFi.status() == WL_CONNECTED)
    {
        display.println("Connected!");
        display.println("IP:");
        display.println(WiFi.localIP());
        display.println("MAC:");
        display.println(WiFi.macAddress());
        setStatus(2, 1);
    }
    else
    {
        display.println("ERROR");
        setStatus(2, 0);
    }
    display.update();
}

void MimirTesting::readSensors()
{
    busyLED(1);
    temp1 = (float)sht31_L.readTemperature();
    temp2 = (float)sht31_H.readTemperature();
    temp3 = (float)bme280.temp();
    temp4 = (float)ccs811B.calculateTemperature();
    hum1 = (float)sht31_L.readHumidity();
    hum2 = (float)sht31_H.readHumidity();
    hum3 = (float)bme280.hum();
    pres = (float)bme280.pres();
    lux1 = (float)veml6030.readLight();
    lux2 = (float)readBH1715(addrBH1715_H);
    lux3 = (float)readBH1715(addrBH1715_L);
    lux4 = analogRead(32) * 3300.0 / 2023.0;
    ccs811B.readData();
    eCO2 = (float)ccs811B.geteCO2();
    tVOC = (float)ccs811B.getTVOC();
}

void MimirTesting::printSensors()
{
    busyLED(1);
    delay(2000);
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    //printValue(temp1, "T1", "C");
    printValue(temp2, "T2", "C");
    printValue(temp3, "T3", "C");
    printValue(temp4, "T4", "C");
    //printValue(hum1, "H1", "%");
    printValue(hum2, "H2", "%");
    printValue(hum3, "H3", "%");
    printValue(pres, "P", "hPa");
    printValue(lux1, "L1", "lux");
    //printValue(lux2, "L2", "lux");
    //printValue(lux3, "L3", "lux");
    printValue(lux4, "L4", "lux");
    printValue(eCO2, "CO2", "ppm");
    printValue(tVOC, "VOC", "ppm");
    display.update();
}

void MimirTesting::sendData()
{
busyLED(3);
    if ((WiFi.status() == WL_CONNECTED))
    {
        String package = packageJSON();

        HTTPClient http;
        http.begin("https://us-central1-mimirhome-app.cloudfunctions.net/sensorData/add");
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(package);
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(2, 20);
        if (httpResponseCode > 0)
        {
            String response = http.getString();
            display.println("Data Sent!");
            display.println(httpResponseCode);
            display.println(response);
            setStatus(3, 1);
        }
        else
        {
            display.println("ERROR");
            display.println("Error on sending POST request");
            display.println(httpResponseCode);
            setStatus(3, 0);
        }
        getIPAddress();
        display.update();

        http.end();
    }
}

void MimirTesting::testBattery()
{
    float voltage = (float)analogRead(12) / 4096 * 5;

    display.setCursor(2, 180);
    display.println("Battery = ");
    display.print(voltage, 2);
    display.println("V");
    display.update();
}

void MimirTesting::testHTTPRequest()
{

    if ((WiFi.status() == WL_CONNECTED))
    {
        HTTPClient http;
        http.begin("https://us-central1-mimirhome-app.cloudfunctions.net/sensorData/test");
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.GET();
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(2, 20);
        if (httpResponseCode > 0)
        {
            String response = http.getString();
            display.println("Data Sent!");
            display.println(httpResponseCode);
            display.println(response);
            setStatus(3, 1);
        }
        else
        {
            display.println("ERROR");
            display.println("Error on sending POST request");
            display.println(httpResponseCode);
            setStatus(3, 0);
        }
        getIPAddress();
        display.update();

        http.end();
    }
}

void MimirTesting::getIPAddress()
{
    if ((WiFi.status() == WL_CONNECTED))
    {
        HTTPClient http2;

        http2.begin("http://api.ipify.org/");
        int httpResponseCode = http2.GET();

        if (httpResponseCode >= 0)
        {
            String response = http2.getString();
            _IP_ADDRESS = response;
            display.println(httpResponseCode);
            display.println(response);
        }
        else
        {
            display.println("Error on sending GET request");
            display.println(httpResponseCode);
        }
        http2.end();
    }
}

void MimirTesting::i2cScanner()
{
    byte error, address;
    int nDevices;

    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    display.println("Scanning...");
    display.update();
    delay(1000);
    display.fillScreen(GxEPD_WHITE);

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            display.print("Device at 0x");
            if (address < 16)
                display.print("0");
            display.println(address, HEX);
            nDevices++;
        }
        else if (error == 4)
        {
            display.print("ERROR at 0x");
            if (address < 16)
                display.print("0");
            display.println(address, HEX);
        }
    }
    if (nDevices == 0)
        display.println("No I2C devices found\n");
    else
        display.println("done\n");

    display.update();
    setStatus(1, 2);
}

void MimirTesting::testNeoPixels(int repeat, int _delay)
{
    for (int j = 0; j > repeat; j++)
    {
        for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256)
        {
            for (int i = 0; i < strip.numPixels(); i++)
            {
                int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
                strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
            }
            strip.show();
            delay(_delay);
        };
    };
    for (int x = 0; x > strip.numPixels(); x++)
    {
        strip.setPixelColor(x, strip.Color(0, 0, 0));
    }
    strip.show();
}

void MimirTesting::currentStatus()
{
    strip.clear();
    strip.setPixelColor(0, statusColour(_BATTERY));
    strip.setPixelColor(1, statusColour(_SENSOR));
    strip.setPixelColor(2, statusColour(_WIFI));
    strip.setPixelColor(3, statusColour(_SERVER));
    strip.setPixelColor(4, statusColour(_MICROSD));
    strip.show();
    delay(500);
    strip.clear();
    strip.show();
}

uint32_t MimirTesting::statusColour(int status)
{
    switch (status)
    {
    case 0:
        return Colour(255, 0, 0);
    case 1:
        return Colour(0, 255, 0);
    case 2:
        return Colour(255, 0, 255);
    default:
        break;
    }
}

void MimirTesting::setStatus(int status, int newStatus)
{
    switch (status)
    {
    case 0:
        _BATTERY = newStatus;
        return currentStatus();
    case 1:
        _SENSOR = newStatus;
        return currentStatus();
    case 2:
        _WIFI = newStatus;
        return currentStatus();
    case 3:
        _SERVER = newStatus;
        return currentStatus();
    case 4:
        _MICROSD = newStatus;
        return currentStatus();
    default:
        return currentStatus();
    }
}

void MimirTesting::writeFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        Serial.println("File written");
    }
    else
    {
        Serial.println("Write failed");
    }
    file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void MimirTesting::appendFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message))
    {
        Serial.println("Message appended");
    }
    else
    {
        Serial.println("Append failed");
    }
    file.close();
}

void MimirTesting::burnReading(int repeat)
{
    for (int i = 0; i > repeat; i++)
    {
        analogRead(32);
    }
}

void MimirTesting::printValue(float value, const char *type, const char *unit)
{
    display.print(type);
    display.print(": ");
    !isnan(value) ? display.print(value, 2)
                  : display.print("ERROR");
    display.println(unit);
}

bool MimirTesting::beginBH1715(uint8_t addr, TwoWire &wirePort)
{
    TwoWire *_i2cPort = &wirePort;

    _i2cPort->beginTransmission(addr);
    _i2cPort->write(0x01);
    _i2cPort->endTransmission();
    delay(6);

    //Set to Continuous Mode
    _i2cPort->beginTransmission(addr);
    _i2cPort->write(0x10);
    uint8_t _ret = _i2cPort->endTransmission();
    if (!_ret)
        return true;
    else
        return false;
}

float MimirTesting::readBH1715(int addr)
{
    unsigned int data[2];
    Wire.requestFrom(addr, 2);
    if (Wire.available() == 2)
    {
        data[0] = Wire.read();
        data[1] = Wire.read();
    }
    delay(300);
    return ((data[0] * 256) + data[1]) / 1.2;
}

String MimirTesting::packageJSON()
{
    if (_IP_ADDRESS == "" || _IP_ADDRESS == "400 Bad Request")
        getIPAddress();

    DynamicJsonDocument package(1024);
    String output;

    JsonObject userInfo = package.createNestedObject("userInfo");
    userInfo["userName"] = _USER;
    userInfo["userId"] = _USER_ID;
    userInfo["deviceId"] = _DEVICE_ID;
    userInfo["ipAddress"] = _IP_ADDRESS;

    JsonObject status = package.createNestedObject("status");
    status["Battery Status"] = _BATTERY;
    status["Sensors Status"] = _SENSOR;
    status["WiFI Status"] = _WIFI;
    status["Server Status"] = _SERVER;
    status["MicroSD Status"] = _MICROSD;

    JsonObject data = package.createNestedObject("data");

    data["Temperature (SHT31_L)"] = temp1;
    data["Temperature (SHT31_H)"] = temp2;
    data["Temperature (BME280)"] = temp3;
    data["Humidity (SHT31_L)"] = hum1;
    data["Humidity (SHT31_H)"] = hum2;
    data["Humidity (BME280)"] = hum3;
    data["Pressure (BME280)"] = pres;
    data["Luminance (VEML6030)"] = lux1;
    data["Luminance (BH1715_H)"] = lux2;
    data["Luminance (BH1715_L)"] = lux3;
    data["Luminance (TEMT600)"] = lux4;
    data["eCO2 (CCS811)"] = eCO2;
    data["VOC (CCS811)"] = tVOC;

    serializeJsonPretty(package, output);
    return output;
}

void MimirTesting::busyLED(int led, int repeat, int duration)
{
    for (int x = 0; x > repeat; x++)
    {
        strip.setPixelColor(led, strip.Color(0, 0, 122));
        strip.show();
        delay(duration / 2);
        strip.setPixelColor(led, strip.Color(0, 0, 0));
        strip.show();
        delay(duration / 2);
    }
}