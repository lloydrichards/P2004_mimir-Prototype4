#include "MimirTesting.h"
#include <FS.h> //this needs to be first, or it all crashes and burns...
#include "SPIFFS.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "time.h"

SPIClass spiSD(HSPI);

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

//Sensor Libraries
#include "Adafruit_SHT31.h"                         //https://github.com/adafruit/Adafruit_SHT31
#include "SparkFun_VEML6030_Ambient_Light_Sensor.h" //https://github.com/sparkfun/SparkFun_Ambient_Light_Sensor_Arduino_Library
#include "SparkFun_VEML6075_Arduino_Library.h"
#include <Adafruit_BMP280.h>
#include "ccs811.h" //https://github.com/maarten-pennings/CCS811
#include <HSCDTD008A.h>

//I2C Addresses
#define addrSHT31D_L 0x44
#define addrSHT31D_H 0x45
#define addrVEML6030 0x48
#define addrVEML6075 0x10
#define addrCCS811 0x5A
#define addrbmp280 0x76
#define addrCompass 0X0C

#include <NeoPixelBrightnessBus.h>

#include <GxEPD.h>
#include <GxGDEH0213B73/GxGDEH0213B73.h>
#include <Fonts/FreeSans9pt7b.h>
#include "epaper_fonts.h"
#include "ui_icons_24.h"
#include "ui_icons_36.h"
#include "ui_icons_48.h"
#include "ui_icons_64.h"
#include "mimir_splash.h"

//Set Screen SPI
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/17, /*RST=*/16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/16, /*BUSY=*/4);

#define BATTERY_SENSOR_PIN 35
#define PIXEL_PIN 19
#define PIXEL_COUNT 5

//Define Sensors
Adafruit_SHT31 sht31_L = Adafruit_SHT31();
Adafruit_SHT31 sht31_H = Adafruit_SHT31();
SparkFun_Ambient_Light veml6030(addrVEML6030);
VEML6075 veml6075;
CCS811 ccs811(36);
Adafruit_BMP280 bmp280;
HSCDTD008A compass(Wire, addrCompass);

//VEML6030 settings
float gain = .125;
int integTime = 100;

//bmp280 settings
#define SEALEVELPRESSURE_HPA (1013.25)

//SD setting
int ID;
String dataMessage;

NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> pixel(PIXEL_COUNT, PIXEL_PIN);

RgbColor red(128, 0, 0);
RgbColor green(0, 0, 128);
RgbColor blue(64, 64, 0);
RgbColor yellow(0, 128, 0);
RgbColor purple(64, 0, 64);
RgbColor lightBlue(0, 64, 64);
RgbColor white(128);
RgbColor black(0);

MimirTesting::MimirTesting()
{
}

void MimirTesting::initDisplay(int baudRate)
{
    display.init(baudRate);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&Lato_Regular_10);
    display.update();
    delay(1000);
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, true);
}

void MimirTesting::initNeoPixels(int brightness)
{
    pixel.Begin();
    pixel.Show();
    pixel.SetBrightness(128);
    //testNeoPixels();
}

void MimirTesting::initSensors(bool _display)
{
    if (sht31_L.begin(addrSHT31D_L))
    {
        SHT31D_L_STATUS = SUCCESS;
    }
    else
        SHT31D_L_STATUS = ERROR_UNDEFINED;

    if (sht31_H.begin(addrSHT31D_H))
    {
        SHT31D_H_STATUS = SUCCESS;
    }
    else
        SHT31D_H_STATUS = ERROR_UNDEFINED;

    if (veml6030.begin())
    {
        veml6030.setGain(gain);
        veml6030.setIntegTime(integTime);
        VEML6030_STATUS = SUCCESS;
    }
    else
        VEML6030_STATUS = ERROR_UNDEFINED;

    if (veml6075.begin())
    {
        VEML6075_STATUS = SUCCESS;
    }
    else
        VEML6075_STATUS = ERROR_UNDEFINED;

    if (ccs811.begin())
    {
        ccs811.start(CCS811_MODE_60SEC);
        CCS811_STATUS = SUCCESS;
    }
    else
        CCS811_STATUS = ERROR_UNDEFINED;

    if (compass.begin())
    {
        compass.calibrate();
        COMPASS_STATUS = SUCCESS;
    }
    else
        COMPASS_STATUS = ERROR_UNDEFINED;

    if (bmp280.begin(addrbmp280))
    {
        BMP280_STATUS = SUCCESS;
    }
    else
        BMP280_STATUS = ERROR_UNDEFINED;

    if (
        SHT31D_L_STATUS == SUCCESS &&
        SHT31D_H_STATUS == SUCCESS &&
        VEML6030_STATUS == SUCCESS &&
        VEML6075_STATUS == SUCCESS &&
        CCS811_STATUS == SUCCESS &&
        BMP280_STATUS == SUCCESS &&
        COMPASS_STATUS == SUCCESS)
    {
        SENSOR_STATUS = SUCCESS;
    }
    else
        SENSOR_STATUS = ERROR_UNDEFINED;

    if (_display)
        DisplaySensors();
}

void MimirTesting::DisplaySensors()
{
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    (SHT31D_L_STATUS != SUCCESS) ? display.println("SHT31_L: X ")
                                 : display.println("SHT31_L: O");
    (SHT31D_H_STATUS != SUCCESS) ? display.println("SHT31_H: X")
                                 : display.println("SHT31_H: O");
    (VEML6030_STATUS != SUCCESS) ? display.println("VEML6030: X")
                                 : display.println("VEML6030: O");
    (VEML6075_STATUS != SUCCESS) ? display.println("VEML6075: X")
                                 : display.println("VEML6075: O");
    (CCS811_STATUS != SUCCESS) ? display.println("CCS811: X")
                               : display.println("CCS811: O");
    (COMPASS_STATUS != SUCCESS) ? display.println("CCS811: X")
                                : display.println("CCS811: O");
    (BMP280_STATUS != SUCCESS) ? display.println("bmp280: X")
                               : display.println("bmp280: O");

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

    wifiManager.autoConnect("mimirAP");

    if (_display)
        DisplayWiFiCredentials();
    if (WiFi.status() == WL_CONNECTED)
    {
        WIFI_STATUS = SUCCESS;
        wifi_signal = WiFi.RSSI();
        SetupTime();
    }
    else
        WIFI_STATUS = ERROR_UNDEFINED;

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

void MimirTesting::initMicroSD(bool _display)
{
    spiSD.begin(14, 2, 15, 13);
    Serial.println("Initializing SD card...");
    if (!SD.begin(13, spiSD))
    {
        Serial.println("ERROR - SD card initialization failed!");
        MICROSD_STATUS = ERROR_UNDEFINED;
        return; // init failed
    }
    // If the data.txt file doesn't exist
    // Create a file on the SD card and write the data labels
    File file = SD.open(filename);
    if (!file)
    {
        Serial.println("File doens't exist");
        Serial.println("Creating file...");
        writeFile(SD, filename, "Date,Time,Battery(%),Temperature(SHT31_L),Temperature(SHT31_H),Temperature(BMP280),Altitude(BMP280),Humidity(SHT31_L),Humidity(SHT31_H),Pressure(BMP280),Luminance(VEML6030),UVA(VEML6075),UVB(VEML6075),UVIndex(VEML6075),eCO2(CCS811),tVOC(CCS811),bearing(Compass); \r\n");
    }
    else
    {
        Serial.println("File already exists");
    }
    file.close();
    MICROSD_STATUS = SUCCESS;
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
};
void MimirTesting::WiFi_OFF()
{
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
};

void MimirTesting::SLEEP()
{
    long SleepTimer = (SleepDuration * 60 - ((CurrentMin % SleepDuration) * 60 + CurrentSec)) + 30; //Some ESP32 are too fast to maintain accurate time
    esp_sleep_enable_timer_wakeup(SleepTimer * 1000000LL);

    Serial.println("Entering " + String(SleepTimer) + "-secs of sleep time");
    Serial.println("Awake for : " + String((millis() - StartTime) / 1000.0, 3) + "-secs");
    Serial.println("Starting deep-sleep...");

    delay(100);
    display.powerDown();

    delay(100);
    esp_deep_sleep_start();
}

void MimirTesting::WAKEUP_REASON()
{
    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
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
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
}

void MimirTesting::readSensors(bool _display)
{
    uint16_t eco2, etvoc, errstat, raw;

    temp1 = (float)sht31_L.readTemperature();
    temp2 = (float)sht31_H.readTemperature();
    temp3 = (float)bmp280.readTemperature();
    hum1 = (float)sht31_L.readHumidity();
    hum2 = (float)sht31_H.readHumidity();
    pres = (float)bmp280.readPressure() / 100;
    alt = (float)bmp280.readAltitude(SEALEVELPRESSURE_HPA);
    lux = (float)veml6030.readLight();
    uvA = (float)veml6075.uva();
    uvB = (float)veml6075.uvb();
    uvIndex = (float)veml6075.index();
    ccs811.read(&eco2, &etvoc, &errstat, &raw);
    eCO2 = (float)eco2;
    tVOC = (float)etvoc;

    if (compass.measure())
    {
        compassX = compass.x();
        compassY = compass.y();
        compassZ = compass.z();
        bearing = ((atan2(compassY, compassX)) * 180) / PI; //values will range from +180 to -180 degrees
    }

    // Serial.print("CCS811: ");
    // if (errstat == CCS811_ERRSTAT_OK)
    // {
    //     Serial.print("eco2=");
    //     Serial.print(eco2);
    //     Serial.print(" ppm  ");
    //     Serial.print("etvoc=");
    //     Serial.print(etvoc);
    //     Serial.print(" ppb  ");
    // }
    // else if (errstat == CCS811_ERRSTAT_OK_NODATA)
    // {
    //     Serial.print("waiting for (new) data");
    // }
    // else if (errstat & CCS811_ERRSTAT_I2CFAIL)
    // {
    //     Serial.print("I2C error");
    // }
    // else
    // {
    //     Serial.print("error: ");
    //     Serial.print(ccs811.errstat_str(errstat));
    // }
    // Serial.println();

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
    (SHT31D_L_STATUS != SUCCESS) ? display.println("SHT31_L: X ")
                                 : display.println("SHT31_L: O");

    (SHT31D_H_STATUS != SUCCESS) ? display.println("SHT31_H: X")
                                 : display.println("SHT31_H: O");

    (VEML6030_STATUS != SUCCESS) ? display.println("VEML6030: X")
                                 : display.println("VEML6030: O");

    (VEML6075_STATUS != SUCCESS) ? display.println("VEML6075: X")
                                 : display.println("VEML6075: O");

    (CCS811_STATUS != SUCCESS) ? display.println("CCS811: X")
                               : display.println("CCS811: O");
    (COMPASS_STATUS != SUCCESS) ? display.println("Compass: X")
                                : display.println("Compass: O");

    (BMP280_STATUS != SUCCESS) ? display.println("bmp280: X")
                               : display.println("bmp280: O");

    display.println("____________");
    display.print("Battery: ");
    display.println(BATTERY_STATUS);
    display.print("Sensors: ");
    display.println(SENSOR_STATUS);
    display.print("WiFi: ");
    display.println(WIFI_STATUS);
    display.print("Server: ");
    display.println(SERVER_STATUS);
    display.print("MicroSD: ");
    display.println(MICROSD_STATUS);

    display.update();
}

void MimirTesting::DisplayReadings()
{
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(2, 20);
    if (SHT31D_L_STATUS == SUCCESS)
    {
        printValue(temp1, "T1", "C");
        printValue(hum1, "H1", "%");
        display.println("____________");
    }

    if (SHT31D_H_STATUS == SUCCESS)
    {
        printValue(temp2, "T2", "C");
        printValue(hum2, "H2", "%");
        display.println("____________");
    }

    if (VEML6030_STATUS == SUCCESS)
    {
        printValue(lux, "L1", "lux");
        display.println("____________");
    }

    if (VEML6075_STATUS == SUCCESS)
    {
        printValue(uvA, "UVA", "");
        printValue(uvB, "UVB", "");
        printValue(uvIndex, "UV Index", "");
        display.println("____________");
    }

    if (CCS811_STATUS == SUCCESS)
    {
        printValue(eCO2, "CO2", "ppm");
        printValue(tVOC, "VOC", "ppb");
        display.println("____________");
    }

    if (BMP280_STATUS == SUCCESS)
    {
        printValue(temp3, "T3", "C");
        printValue(pres, "P", "hPa");
        printValue(alt, "Alt", "m");
        display.println("____________");
    }

    if (COMPASS_STATUS == SUCCESS)
    {
        printValue(bearing, "Bearing", "deg");
        printValue(compassX, "X", "deg");
        printValue(compassY, "Y", "deg");
        printValue(compassZ, "Z", "deg");
    }

    display.update();
}

void MimirTesting::sendData(bool _display)
{
    if ((WiFi.status() == WL_CONNECTED))
    {
        SetupTime();
        String package = packageJSON();

        HTTPClient http;
        http.begin("https://us-central1-mimirhome-app.cloudfunctions.net/sensorData/add");
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(package);
        String response = http.getString();

        if (_display)
            DisplaySentData(httpResponseCode, response);

        if (httpResponseCode > 0)
        {
            SERVER_STATUS = SUCCESS;
            display.println("Data Sent!");
            display.println(httpResponseCode);
            display.println(response);
            blinkPixel(1, 0, 255, 0, 2);
        }
        else
        {
            SERVER_STATUS = ERROR_WRITE;
            display.println("ERROR");
            display.println("Error on sending POST request");
            display.println(httpResponseCode);
            blinkPixel(1, 255, 0, 0, 2);
        }
        getIPAddress();

        http.end();
    }
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
            SERVER_STATUS = SUCCESS;
            String response = http.getString();
            display.println("Data Sent!");
            display.println(httpResponseCode);
            display.println(response);
        }
        else
        {
            SERVER_STATUS = ERROR_READ;
            display.println("ERROR");
            display.println("Error on sending GET request");
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
    for (int i = 0; i < PIXEL_COUNT; i++)
    {
        pixel.SetPixelColor(i, red);
        pixel.Show();
        delay(500);
        pixel.SetPixelColor(i, black);
        pixel.Show();
    }
    for (int i = 0; i < PIXEL_COUNT; i++)
    {
        pixel.SetPixelColor(i, green);
        pixel.Show();
        delay(500);
        pixel.SetPixelColor(i, black);
        pixel.Show();
    }
    for (int i = 0; i < PIXEL_COUNT; i++)
    {
        pixel.SetPixelColor(i, blue);
        pixel.Show();
        delay(500);
        pixel.SetPixelColor(i, black);
        pixel.Show();
    }
    for (int i = 0; i < PIXEL_COUNT; i++)
    {
        pixel.SetPixelColor(i, yellow);
        pixel.Show();
        delay(500);
        pixel.SetPixelColor(i, black);
        pixel.Show();
    }
    for (int i = 0; i < PIXEL_COUNT; i++)
    {
        pixel.SetPixelColor(i, purple);
        pixel.Show();
        delay(500);
        pixel.SetPixelColor(i, black);
        pixel.Show();
    }
    for (int i = 0; i < PIXEL_COUNT; i++)
    {
        pixel.SetPixelColor(i, lightBlue);
        pixel.Show();
        delay(500);
        pixel.SetPixelColor(i, black);
        pixel.Show();
    }
}

void MimirTesting::logData(bool display)
{

    dataMessage = String(DateStr) + "," +
                  String(TimeStr) + "," +
                  String(batteryPercent) + "," +
                  String(temp1) + "," +
                  String(temp2) + "," +
                  String(temp3) + "," +
                  String(alt) + "," +
                  String(hum1) + "," +
                  String(hum2) + "," +
                  String(pres) + "," +
                  String(lux) + "," +
                  String(uvA) + "," +
                  String(uvB) + "," +
                  String(uvIndex) + "," +
                  String(eCO2) + "," +
                  String(tVOC) + "," +
                  String(bearing) + "\r\n";

    Serial.print("Save data: ");
    Serial.println(dataMessage);

    File file = SD.open(filename);
    if (!file)
    {
        Serial.println("File doens't exist");
        Serial.println("Creating file...");
        writeFile(SD, filename, "Date,Time,Battery(%),Temperature(SHT31_L),Temperature(SHT31_H),Temperature(BMP280),Altitude(BMP280),Humidity(SHT31_L),Humidity(SHT31_H),Pressure(BMP280),Luminance(VEML6030),UVA(VEML6075),UVB(VEML6075),UVIndex(VEML6075),eCO2(CCS811),tVOC(CCS811),bearing(Compass); \r\n");
    }
    file.close();
    appendFile(SD, filename, dataMessage.c_str());
}

void MimirTesting::writeFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        MICROSD_STATUS = ERROR_UNDEFINED;
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        MICROSD_STATUS = SUCCESS;
        Serial.println("File written");
    }
    else
    {
        MICROSD_STATUS = ERROR_WRITE;
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
        MICROSD_STATUS = ERROR_UNDEFINED;
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message))
    {
        MICROSD_STATUS = SUCCESS;
        Serial.println("Message appended");
    }
    else
    {
        MICROSD_STATUS = ERROR_WRITE;
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
    status["Battery_Status"] = BATTERY_STATUS;
    status["Sensors_Status"] = SENSOR_STATUS;
    status["WiFI_Status"] = WIFI_STATUS;
    status["Server_Status"] = SERVER_STATUS;
    status["MicroSD_Status"] = MICROSD_STATUS;
    status["Battery_Percent"] = batteryPercent;
    status["WiFi_Signal"] = wifi_signal;
    status["Date"] = DateStr;
    status["Time"] = TimeStr;

    JsonObject data = package.createNestedObject("data");

    data["Temperature(SHT31_L)"] = temp1;
    data["Temperature(SHT31_H)"] = temp2;
    data["Temperature(bmp280)"] = temp3;
    data["Humidity(SHT31_L)"] = hum1;
    data["Humidity(SHT31_H)"] = hum2;
    data["Pressure(BMP280)"] = pres;
    data["Altitude(BMP280)"] = pres;
    data["Luminance(VEML6030)"] = lux;
    data["UVA(VEML6075)"] = uvA;
    data["UVB(VEML6075)"] = uvB;
    data["UVIndex(VEML6075)"] = uvIndex;
    data["eCO2(CCS811)"] = eCO2;
    data["VOC(CCS811)"] = tVOC;
    data["Bearing(Compass)"] = bearing;

    serializeJsonPretty(package, output);
    return output;
}

void MimirTesting::blinkPixel(int pixel, int R, int G, int B, int repeat)
{
}

void MimirTesting::readBattery(bool _display)
{
    pinMode(BATTERY_SENSOR_PIN, INPUT);
    float voltage = getBatteryVoltage(); //output value
    const float battery_max = 4.2;       //maximum voltage of battery
    const float battery_min = 3.3;       //minimum voltage of battery before shutdown
    batteryPercent = roundf(((voltage - battery_min) / (battery_max - battery_min)) * 100);

    if (batteryPercent < 10)
    {
        BATTERY_STATUS = CRITICAL_BATTERY;
    }
    else if (batteryPercent < 20)
    {
        BATTERY_STATUS = LOW_BATTERY;
    }
    else if (batteryPercent < 95)
    {
        BATTERY_STATUS = GOOD_BATTERY;
    }
    else if (batteryPercent <= 100)
    {
        BATTERY_STATUS = FULL_BATTERY;
    }
    else if (batteryPercent > 100)
    {
        BATTERY_STATUS = CHARGING;
    }

    display.setCursor(0, 50);
    display.println("Battery Level...");
    printValue(voltage, "Voltage", "V");
    printValue(batteryPercent, "Battery", "%");

    DisplayBatteryIcon(20, 100);
    DisplayWiFiIcon(50, 200);

    display.updateWindow(0, 40, GxEPD_WIDTH, GxEPD_HEIGHT - 40);
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
    strftime(day_output, 30, "%F", &timeinfo);               // Displays: Sat 24-Jun-17
    strftime(output, sizeof(output), "%H:%M:%S", &timeinfo); // Creates: '14:05:49'
    createFileName(day_output);
    DateStr = day_output;
    TimeStr = output;
    return true;
}

void MimirTesting::createFileName(char date[])
{
    filename[1] = date[0];
    filename[2] = date[1];
    filename[3] = date[2];
    filename[4] = date[3];
    filename[5] = date[4];
    filename[6] = date[5];
    filename[7] = date[6];
    filename[8] = date[7];
    filename[9] = date[8];
    filename[10] = date[9];
}

void MimirTesting::busyNeoPixels(){

};
void MimirTesting::statusNeoPixels(int _delay)
{
    pixel.SetPixelColor(BATTERY_LED, getBatteryColor(BATTERY_STATUS));
    pixel.SetPixelColor(MICROSD_LED, getStatusColor(MICROSD_STATUS));
    pixel.SetPixelColor(SENSOR_LED, getStatusColor(SENSOR_STATUS));
    pixel.SetPixelColor(SERVER_LED, getStatusColor(SERVER_STATUS));
    pixel.SetPixelColor(WIFI_LED, getStatusColor(WIFI_STATUS));
    pixel.Show();
    delay(_delay);
    pixel.SetPixelColor(BATTERY_LED, black);
    pixel.SetPixelColor(MICROSD_LED, black);
    pixel.SetPixelColor(SENSOR_LED, black);
    pixel.SetPixelColor(SERVER_LED, black);
    pixel.SetPixelColor(WIFI_LED,black);
    pixel.Show();
    
};
void MimirTesting::activeNeoPixels(STATUS_LED system, uint32_t colour, int repeat){

};

RgbColor MimirTesting::getStatusColor(enum STATUS_ERROR STATUS)
{
    switch (STATUS)
    {
    case ERROR_READ:
        return red;
    case ERROR_WRITE:
        return red;
    case ERROR_UNDEFINED:
        return red;
    case UNMOUNTED:
        return yellow;
    case SUCCESS:
        return green;
    default:
        return black;
    }
}

RgbColor MimirTesting::getBatteryColor(enum STATUS_BATTERY STATUS)
{
    switch (STATUS)
    {
    case CRITICAL_BATTERY:
        return red;
    case LOW_BATTERY:
        return yellow;
    case GOOD_BATTERY:
        return lightBlue;
    case FULL_BATTERY:
        return green;
    case CHARGING:
        return purple;
    default:
        return black;
    }
}