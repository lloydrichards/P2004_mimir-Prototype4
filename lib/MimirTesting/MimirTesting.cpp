#include "MimirTesting.h"
#include <FS.h> //this needs to be first, or it all crashes and burns...
#include "SPIFFS.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "time.h"

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

//Sensor Libraries
#include "Adafruit_SHT31.h"                         //https://github.com/adafruit/Adafruit_SHT31
#include "SparkFun_VEML6030_Ambient_Light_Sensor.h" //https://github.com/sparkfun/SparkFun_Ambient_Light_Sensor_Arduino_Library
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
#include "epaper_fonts.h"

//Set Screen SPI
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/17, /*RST=*/16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/16, /*BUSY=*/4);

#define BATTERY_SENSOR_PIN 35
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
    display.setTextColor(GxEPD_BLACK);
    display.setRotation(0);
    display.setCursor(0, 20);

    display.setFont(&Lato_Regular_10);
    display.print("Temp: ");
    display.setFont(&Roboto_Slab_Regular_16);
    display.print("21.6");
    display.println(" C");
    display.println();

    display.setFont(&Lato_Regular_10);
    display.print("Humi: ");
    display.setFont(&Roboto_Slab_Regular_16);
    display.print("37");
    display.println(" %");
    display.println();

    display.setFont(&Lato_Regular_10);
    display.print("Pres: ");
    display.setFont(&Roboto_Slab_Regular_16);
    display.print("2199");
    display.println(" Hpa");
    display.println();

    display.setFont(&Lato_Regular_10);
    display.print("CO2: ");
    display.setFont(&Roboto_Slab_Regular_16);
    display.print("1200");
    display.println(" ppm");
    display.println();

    display.setFont(&Lato_Regular_10);
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

void MimirTesting::initSensors(bool _display)
{
    SHT31D_L_STATUS = sht31_L.begin(addrSHT31D_L);
    SHT31D_H_STATUS = sht31_H.begin(addrSHT31D_H);
    if (veml6030.begin())
    {
        veml6030.setGain(gain);
        veml6030.setIntegTime(integTime);
        VEML6030_STATUS = true;
    }
    pinMode(32, INPUT);
    TEMT600_STATUS = !isnan(analogRead(32));
    CCS811B_STATUS = ccs811B.begin();
    BMP280_STATUS = bmp280.begin(addrbmp280);

    if (_display)
        DisplaySensors();
}

void MimirTesting::DisplaySensors()
{
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    !SHT31D_L_STATUS ? display.println("SHT31_L: X ")
                     : display.println("SHT31_L: O");

    !SHT31D_H_STATUS ? display.println("SHT31_H: X")
                     : display.println("SHT31_H: O");

    !VEML6030_STATUS ? display.println("VEML6030: X")
                     : display.println("VEML6030: O");

    !CCS811B_STATUS ? display.println("CCS811B: X")
                    : display.println("CCS811B: O");

    !BMP280_STATUS ? display.println("bmp280: X")
                   : display.println("bmp280: O");

    !TEMT600_STATUS ? display.println("TEMT600: O")
                    : display.println("TEMT600: X");

    display.update();
}

void MimirTesting::initWIFI(bool _display)
{
    WiFiManagerParameter custom_USER("User Name", "User Name", _USER, 40);
    WiFiManagerParameter custom_USER_ID("User ID", "User ID", _USER_ID, 40);
    WiFiManagerParameter custom_DEVICE_ID("Device ID", "Device ID", _DEVICE_ID, 40);

    WiFiManager wifiManager;

    wifiManager.addParameter(&custom_USER);
    wifiManager.addParameter(&custom_USER_ID);
    wifiManager.addParameter(&custom_DEVICE_ID);

    strip.setPixelColor(2, strip.Color(255, 255, 0));
    strip.show();
    wifiManager.autoConnect("mimirAP");
    strip.setPixelColor(2, strip.Color(0, 255, 0));
    strip.show();
    if (_display)
        DisplayWiFiCredentials();
    if (WiFi.status() == WL_CONNECTED)
    {
        wifi_signal = WiFi.RSSI();
        SetupTime();
    }
    else
    {
        strip.setPixelColor(2, strip.Color(255, 0, 0));
        strip.show();
    }

    //Update Device Info with Params
    strcpy(_USER, custom_USER.getValue());
    strcpy(_USER_ID, custom_USER_ID.getValue());
    strcpy(_DEVICE_ID, custom_DEVICE_ID.getValue());

    saveConfig();

    WiFi_OFF();
}
void MimirTesting::initConfig()
{
    if (SPIFFS.begin())
    {

        Serial.println("mounted file system");
        if (SPIFFS.exists("/config.json"))
        {
            //file exists, reading and loading
            Serial.println("reading config file");
            fs::File configFile = SPIFFS.open("/config.json", "r");
            if (configFile)
            {
                Serial.println("opened config file");
                size_t size = configFile.size();
                // Allocate a buffer to store contents of the file.
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);
                DynamicJsonDocument configJson(1024);
                DeserializationError error = deserializeJson(configJson, buf.get());
                if (error)
                {
                    Serial.println("failed to load json config");
                    return;
                }
                Serial.println("\nparsed json");
                serializeJson(configJson, Serial);

                strcpy(_USER, configJson["User"] | "N/A");
                strcpy(_USER_ID, configJson["UserID"] | "N/A");
                strcpy(_DEVICE_ID, configJson["DeviceID"] | "N/A");
            }
            else
            {
                Serial.println("failed to load json config");
            }
        }
    }
    else
    {
        Serial.println("failed to mount FS");
    }
}

void MimirTesting::saveConfig()
{
    DynamicJsonDocument newConfigJson(1024);
    newConfigJson["User"] = _USER;
    newConfigJson["UserID"] = _USER_ID;
    newConfigJson["DeviceID"] = _DEVICE_ID;

    fs::File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
        Serial.println("failed to open config file for writing");
    }

    serializeJson(newConfigJson, Serial);
    serializeJson(newConfigJson, configFile);
    configFile.close();
}

void MimirTesting::DisplayWiFiCredentials()
{
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    if (WiFi.status() == WL_CONNECTED)
    {
        display.println("Connected!");
        display.println("IP:");
        display.println(WiFi.localIP());
        display.println("MAC:");
        display.println(WiFi.macAddress());
        display.println("RSSI:");
        display.println(WiFi.RSSI());
    }
    else
    {
        display.println("ERROR");
    }
    display.update();
}

void MimirTesting::initDash()
{
    display.fillScreen(GxEPD_WHITE);
    DisplayWiFiIcon(70, 20);
    DisplayBatteryIcon(100, 20);
    display.drawLine(0, 25, GxEPD_WIDTH, 25, GxEPD_BLACK);
    display.update();
}

void MimirTesting::initTimer()
{
    StartTime = millis();
}

void MimirTesting::forceStartWiFi()
{
    WiFiManagerParameter custom_USER("User Name", "User Name", _USER, 40);
    WiFiManagerParameter custom_USER_ID("User ID", "User ID", _USER_ID, 40);
    WiFiManagerParameter custom_DEVICE_ID("Device ID", "Device ID", _DEVICE_ID, 40);

    WiFiManager wifiManager;

    wifiManager.addParameter(&custom_USER);
    wifiManager.addParameter(&custom_USER_ID);
    wifiManager.addParameter(&custom_DEVICE_ID);

    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    display.println("*WiFi Config*");
    display.println("****Mode*****");
    display.update();

    if (!wifiManager.startConfigPortal("OnDemandAP"))
    {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        //reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(5000);
    }

    delay(5000);
    //Update Device Info with Params
    strcpy(_USER, custom_USER.getValue());
    strcpy(_USER_ID, custom_USER_ID.getValue());
    strcpy(_DEVICE_ID, custom_DEVICE_ID.getValue());

    saveConfig();

    WiFi_OFF();
}

void MimirTesting::WiFi_ON()
{

    WiFiManager wifiManager;
    wifiManager.autoConnect("mimirAP");
    wifi_signal = WiFi.RSSI();
    strip.setPixelColor(2, strip.Color(0, 255, 0));
    strip.show();
};
void MimirTesting::WiFi_OFF()
{
    strip.setPixelColor(2, strip.Color(255, 255, 0));
    strip.show();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    strip.setPixelColor(2, strip.Color(0, 0, 0));
    strip.show();
};

void MimirTesting::SLEEP()
{
    long SleepTimer = (SleepDuration * 60 - ((CurrentMin % SleepDuration) * 60 + CurrentSec)) + 30; //Some ESP32 are too fast to maintain accurate time
    esp_sleep_enable_timer_wakeup(SleepTimer * 1000000LL);
    display.setCursor(2, 120);
    display.fillRect(0, 100, GxEPD_WIDTH, GxEPD_HEIGHT - 100, GxEPD_WHITE);
    display.updateWindow(0, 100, GxEPD_WIDTH, GxEPD_HEIGHT - 100);
    display.println("Entering " + String(SleepTimer) + "-secs of sleep time");
    display.println("Awake for : " + String((millis() - StartTime) / 1000.0, 3) + "-secs");
    display.println("Starting deep-sleep...");
    display.updateWindow(0, 100, GxEPD_WIDTH, GxEPD_HEIGHT - 100);
    delay(100);
    display.powerDown();

    for (int y = 0; y > strip.numPixels(); y++)
    {
        strip.setPixelColor(y, strip.Color(0, 0, 0));
    }
    strip.show();
    strip.clear();
    delay(100);
    esp_deep_sleep_start();
}

void MimirTesting::readSensors(bool _display)
{
    temp1 = (float)sht31_L.readTemperature();
    temp2 = (float)sht31_H.readTemperature();
    temp3 = (float)bmp280.readTemperature();
    hum1 = (float)sht31_L.readHumidity();
    hum2 = (float)sht31_H.readHumidity();
    pres = (float)bmp280.readPressure() / 100;
    alt = (float)bmp280.readAltitude(SEALEVELPRESSURE_HPA);
    lux1 = (float)veml6030.readLight();
    lux2 = (float)(analogRead(32) * 0.9765625);
    //ccs811B.readData();
    //eCO2 = (float)ccs811B.geteCO2();
    //tVOC = (float)ccs811B.getTVOC();
    if (_display)
        DisplayReadings();
}
void MimirTesting::DisplayDeviceInfo()
{
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(0, 20);
    display.print("Time: ");
    display.println(TimeStr);
    display.println(DateStr);
    display.println("____________");
    display.print("User: ");
    display.println(_USER);
    display.print("UserID: ");
    display.println(_USER_ID);
    display.print("DeviceID: ");
    display.println(_DEVICE_ID);
    display.print("IP: ");
    display.println(_IP_ADDRESS);
    display.println("____________");
    !SHT31D_L_STATUS ? display.println("SHT31_L: X ")
                     : display.println("SHT31_L: O");

    !SHT31D_H_STATUS ? display.println("SHT31_H: X")
                     : display.println("SHT31_H: O");

    !VEML6030_STATUS ? display.println("VEML6030: X")
                     : display.println("VEML6030: O");

    !CCS811B_STATUS ? display.println("CCS811B: X")
                    : display.println("CCS811B: O");

    !BMP280_STATUS ? display.println("bmp280: X")
                   : display.println("bmp280: O");

    !TEMT600_STATUS ? display.println("TEMT600: O")
                    : display.println("TEMT600: X");

    display.println("____________");
    display.print("Battery: ");
    display.println(_BATTERY);
    display.print("Sensors: ");
    display.println(_SENSOR);
    display.print("WiFi: ");
    display.println(_WIFI);
    display.print("Server: ");
    display.println(_SERVER);
    display.print("MicroSD: ");
    display.println(_MICROSD);

    display.update();
}

void MimirTesting::DisplayReadings()
{
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    if (SHT31D_L_STATUS)
    {
        printValue(temp1, "T1", "C");
        printValue(hum1, "H1", "%");
        display.println("____________");
    }

    if (SHT31D_H_STATUS)
    {
        printValue(temp2, "T2", "C");
        printValue(hum2, "H2", "%");
        display.println("____________");
    }

    if (VEML6030_STATUS)
    {
        printValue(lux1, "L1", "lux");
        display.println("____________");
    }

    if (CCS811B_STATUS)
    {
        printValue(eCO2, "CO2", "ppm");
        printValue(tVOC, "VOC", "ppb");
        display.println("____________");
    }

    if (BMP280_STATUS)
    {
        printValue(temp3, "T3", "C");
        printValue(pres, "P", "hPa");
        printValue(alt, "Alt", "m");
        display.println("____________");
    }

    if (TEMT600_STATUS)
    {
        printValue(lux2, "L2", "lux");
    }

    display.update();
}

void MimirTesting::sendData(bool _display)
{
    if ((WiFi.status() == WL_CONNECTED))
    {
        String package = packageJSON();
        blinkPixel(3, 255, 255, 0, 2);

        HTTPClient http;
        http.begin("https://us-central1-mimirhome-app.cloudfunctions.net/sensorData/add");
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(package);
        String response = http.getString();

        if (_display)
            DisplaySentData(httpResponseCode, response);

        if (httpResponseCode > 0)
        {

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

        http.end();
    }
    else
        initWIFI();
}

void MimirTesting::DisplaySentData(int httpResponseCode, String response)
{
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);

    if (httpResponseCode > 0)
    {
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
    data["Luminance (TEMT600)"] = lux2;
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

void MimirTesting::readBattery(bool _display)
{
    pinMode(BATTERY_SENSOR_PIN, INPUT);
    float voltage = getBatteryVoltage(); //output value
    const float battery_max = 4.2;       //maximum voltage of battery
    const float battery_min = 3.3;       //minimum voltage of battery before shutdown
    batteryPercent = roundf(((voltage - battery_min) / (battery_max - battery_min)) * 100);

    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    display.println("Battery Level...");
    printValue(voltage, "Voltage", "V");
    printValue(batteryPercent, "Battery", "%");

    DisplayBatteryIcon(10, 200);
    DisplayWiFiIcon(40, 200);
    display.drawLine(0, 201, 80, 201, GxEPD_BLACK);

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
        sum += analogRead(BATTERY_SENSOR_PIN);
        delayMicroseconds(1000);
    }
    // calculate the voltage
    voltage = sum / (float)500;
    Serial.print("Battery Analog Read: ");
    Serial.println(sum);
    voltage = (voltage * 3.476) / 4096.0; //for internal 1.1v reference
                                          // use if added divider circuit
    voltage = voltage / (R2 / (R1 + R2));
    //round value by two precision
    voltage = roundf(voltage * 100) / 100;
    Serial.print("Voltage: ");
    Serial.println(voltage);
    return voltage;
}

void MimirTesting::DisplayBatteryIcon(int x, int y)
{
    display.drawRect(x, y - 10, 20, 10, GxEPD_BLACK); // Draw battery pack
    display.fillRect(x + 20, y - 7, 3, 5, GxEPD_BLACK);
    if (batteryPercent < 20) //Draw Warning Battery
    {
        display.fillCircle(x + 4, y - 5, 2, GxEPD_BLACK);
        display.fillRect(x + 8, y - 6, 10, 3, GxEPD_BLACK);
    }
    else if (batteryPercent < 100) // Draw Percent Battery
    {
        display.fillRect(x + 2, y - 8, 16 * batteryPercent / 100.0, 6, GxEPD_BLACK);
    }
    else // Draw Charging Battery
    {
        display.drawLine(x + 2, y - 3, x + 10, y - 8, GxEPD_BLACK);
        display.drawLine(x + 10, y - 8, x + 10, y - 3, GxEPD_BLACK);
        display.drawLine(x + 10, y - 3, x + 17, y - 8, GxEPD_BLACK);
    }
}

void MimirTesting::DisplayWiFiIcon(int x, int y)
{
    int WIFIsignallevel = 0;
    int xpos = 1;
    for (int _rssi = -100; _rssi <= wifi_signal; _rssi = _rssi + 20)
    {
        if (_rssi <= -20)
            WIFIsignallevel = 20; //            <-20dbm displays 5-bars
        if (_rssi <= -40)
            WIFIsignallevel = 16; //  -40dbm to  -21dbm displays 4-bars
        if (_rssi <= -60)
            WIFIsignallevel = 12; //  -60dbm to  -41dbm displays 3-bars
        if (_rssi <= -80)
            WIFIsignallevel = 8; //  -80dbm to  -61dbm displays 2-bars
        if (_rssi <= -100)
            WIFIsignallevel = 4; // -100dbm to  -81dbm displays 1-bar
        display.fillRect(x + xpos * 5, y - WIFIsignallevel, 4, WIFIsignallevel, GxEPD_BLACK);
        xpos++;
    }
    display.fillRect(x, y - 1, 4, 1, GxEPD_BLACK);
}

void MimirTesting::drawString(int x, int y, String text, alignment align)
{
    int16_t x1, y1; //the bounds of x,y and w and h of the variable 'text' in pixels.
    uint16_t w, h;
    display.setTextWrap(false);
    display.getTextBounds(text, x, y, &x1, &y1, &w, &h);
    if (align == RIGHT)
        x = x - w;
    if (align == CENTER)
        x = x - w / 2;
    display.setCursor(x, y + h);
    display.print(text);
}

bool MimirTesting::SetupTime()
{
    configTime(0, 0, "ch.pool.ntp.org", "time.nist.gov");
    setenv("TZ", TZ_INFO, 1);
    delay(100);
    bool TimeStatus = UpdateLocalTime();
    return TimeStatus;
}

bool MimirTesting::UpdateLocalTime()
{
    struct tm timeinfo;
    char output[30], day_output[30];
    while (!getLocalTime(&timeinfo, 5000))
    { // Wait for up to 5-secs
        Serial.println(F("Failed to obtain time"));
        return false;
    }
    CurrentHour = timeinfo.tm_hour;
    CurrentMin = timeinfo.tm_min;
    CurrentSec = timeinfo.tm_sec;
    //See http://www.cplusplus.com/reference/ctime/strftime/
    //Serial.println(&timeinfo, "%H:%M:%S");                               // Displays: 14:05:49
    strftime(day_output, 30, "%a  %d-%b-%y", &timeinfo);     // Displays: Sat 24-Jun-17
    strftime(output, sizeof(output), "%H:%M:%S", &timeinfo); // Creates: '14:05:49'
    DateStr = day_output;
    TimeStr = output;
    return true;
}