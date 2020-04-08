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
#include <Adafruit_BMP280.h>
#include "ccs811.h"

//I2C Addresses
#define addrSHT31D_L 0x44
#define addrSHT31D_H 0x45
#define addrBH1715_L 0x23
#define addrBH1715_H 0x5C
#define addrVEML6030 0x10
#define addrCCS811B 0x5A
#define addrbmp280 0x76

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
CCS811 ccs811B(33);
Adafruit_BMP280 bmp280;

//VEML6030 settings
float gain = .125;
int integTime = 100;

//bmp280 settings
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
    strip.setBrightness(brightness);
    strip.show(); // Turn OFF all pixels ASAP
}

void MimirTesting::initSensors()
{

    pinMode(12, INPUT);
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    blinkPixel(1, 0, 0, 255, 2);
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

    !ccs811B.begin() ? display.println("CCS811B: X")
                     : display.println("CCS811B: O");

    !bmp280.begin(addrbmp280) ? display.println("bmp280: X")
                              : display.println("bmp280: O");

    pinMode(36, OUTPUT);
    digitalWrite(36, LOW);
    !beginBH1715(addrBH1715_L) ? display.println("BH1715_L: X")
                               : display.println("BH1715_L: O");
    !beginBH1715(addrBH1715_H) ? display.println("BH1715_H: X")
                               : display.println("BH1715_H: O");
    pinMode(32, INPUT);
    !isnan(analogRead(32)) ? display.println("TEMT600: O")
                           : display.println("TEMT600: X");

    display.update();
}

void MimirTesting::initWIFI()
{
    WiFiManager wifiManager;
    strip.setPixelColor(2, strip.Color(255, 255, 0));
    strip.show();
    wifiManager.autoConnect("mimirAP");
    strip.setPixelColor(2, strip.Color(0, 255, 0));
    strip.show();
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    if (WiFi.status() == WL_CONNECTED)
    {
        display.println("Connected!");
        display.println("IP:");
        display.println(WiFi.localIP());
        display.println("MAC:");
        display.println(WiFi.macAddress());
    }
    else
    {
        display.println("ERROR");
        strip.setPixelColor(2, strip.Color(255, 0, 0));
        strip.show();
    }
    display.update();
    WiFi_OFF();
}

void MimirTesting::forceStartWiFi()
{
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    display.println("*WiFi Config*");
    display.println("****Mode*****");
    display.update();
    delay(5000);
}

void MimirTesting::WiFi_ON()
{

    WiFiManager wifiManager;
    wifiManager.autoConnect("mimirAP");
    strip.setPixelColor(2, strip.Color(0, 255, 0));
    strip.show();
};
void MimirTesting::WiFi_OFF()
{
    WiFi.mode(WIFI_OFF);
    strip.setPixelColor(2, strip.Color(0, 0, 0));
    strip.show();
};

void MimirTesting::SLEEP()
{
    for (int y = 0; y > strip.numPixels(); y++)
    {
        strip.setPixelColor(y, strip.Color(0, 0, 0));
    }
    strip.show();
    delay(100);
}

void MimirTesting::readSensors()
{
    temp1 = (float)sht31_L.readTemperature();
    temp2 = (float)sht31_H.readTemperature();
    temp3 = (float)bmp280.readTemperature();
    hum1 = (float)sht31_L.readHumidity();
    hum2 = (float)sht31_H.readHumidity();
    pres = (float)bmp280.readPressure() / 100;
    alt = (float)bmp280.readAltitude(SEALEVELPRESSURE_HPA);
    lux1 = (float)veml6030.readLight();
    lux2 = (float)readBH1715(addrBH1715_H);
    lux3 = (float)readBH1715(addrBH1715_L);
    lux4 = (float)(analogRead(32) * 0.9765625);
    //ccs811B.readData();
    //eCO2 = (float)ccs811B.geteCO2();
    //tVOC = (float)ccs811B.getTVOC();
    blinkPixel(1, 0, 255, 0, 2);
}

void MimirTesting::printSensors()
{
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    //printValue(temp1, "T1", "C");
    printValue(temp2, "T2", "C");
    printValue(temp3, "T3", "C");

    //printValue(hum1, "H1", "%");
    printValue(hum2, "H2", "%");
    printValue(pres, "P", "hPa");
    printValue(alt, "Alt", "m");
    printValue(lux1, "L1", "lux");
    //printValue(lux2, "L2", "lux");
    //printValue(lux3, "L3", "lux");
    printValue(lux4, "L4", "lux");
    printValue(eCO2, "CO2", "ppm");
    printValue(tVOC, "VOC", "ppb");
    display.update();
}

void MimirTesting::sendData()
{
    if ((WiFi.status() == WL_CONNECTED))
    {
        String package = packageJSON();
        blinkPixel(3, 255, 255, 0, 2);

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
            blinkPixel(1, 0, 255, 0, 2);
        }
        else
        {
            display.println("ERROR");
            display.println("Error on sending POST request");
            display.println(httpResponseCode);
            blinkPixel(1, 255, 0, 0, 2);
        }
        getIPAddress();
        display.update();

        http.end();
    }
    else
        initWIFI();
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
        }
        else
        {
            display.println("ERROR");
            display.println("Error on sending POST request");
            display.println(httpResponseCode);
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

void MimirTesting::printValue(float value, const char *type, const char *unit, int decimal)
{
    display.print(type);
    display.print(": ");
    !isnan(value) ? display.print(value, decimal)
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
    data["Temperature (bmp280)"] = temp3;
    data["Humidity (SHT31_L)"] = hum1;
    data["Humidity (SHT31_H)"] = hum2;
    data["Pressure (bmp280)"] = pres;
    data["Altitude (bmp280)"] = pres;
    data["Luminance (VEML6030)"] = lux1;
    data["Luminance (BH1715_H)"] = lux2;
    data["Luminance (BH1715_L)"] = lux3;
    data["Luminance (TEMT600)"] = lux4;
    data["eCO2 (CCS811)"] = eCO2;
    data["VOC (CCS811)"] = tVOC;

    serializeJsonPretty(package, output);
    return output;
}

void MimirTesting::blinkPixel(int pixel, int R, int G, int B, int repeat)
{
    for (int k = 0; k > repeat; k++)
    {
        strip.setPixelColor(pixel, strip.Color(R, G, B));
        strip.show();
        delay(500);
        strip.setPixelColor(pixel, strip.Color(0, 0, 0));
        strip.show();
        delay(500);
    };
}

void MimirTesting::readBattery()
{
    float voltage = getBatteryVoltage(); //output value
    const float battery_max = 4.2;       //maximum voltage of battery
    const float battery_min = 3.3;       //minimum voltage of battery before shutdown
    float batteryPercent = ((voltage - battery_min) / (battery_max - battery_min)) * 100;
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    display.println("Battery Level...");
    display.print("Voltage: ");
    display.println(voltage);
    display.print("Battery %: ");
    display.println(batteryPercent);
    display.println(analogRead(12));
    display.update();
}

float MimirTesting::getBatteryVoltage()
{
    //read battery voltage per %
    long sum = 0;        // sum of samples taken
    float voltage = 0.0; // calculated voltage

    float R1 = 10000.0; // resistance of R1 (10K)
    float R2 = 10000.0; // resistance of R2 (10K)

    for (int i = 0; i < 500; i++)
    {
        sum += analogRead(12);
        delayMicroseconds(1000);
    }
    // calculate the voltage
    voltage = sum / (float)500;
    voltage = (voltage * 3.6) / 4096.0; //for internal 1.1v reference
                                        // use if added divider circuit
    voltage = voltage / (R2 / (R1 + R2));
    //round value by two precision
    voltage = roundf(voltage * 100) / 100;
    return voltage;
}