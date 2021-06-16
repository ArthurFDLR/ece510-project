// System libraries - From Arduino core for the ESP32
// https://github.com/espressif/arduino-esp32
#include <WiFi.h>
#include <Wire.h>
#include <esp_camera.h>

/***************************************
 *  ESP32 connections
 **************************************/
#define PWDN_GPIO_NUM       -1
#define RESET_GPIO_NUM      -1
#define XCLK_GPIO_NUM       32
#define SIOD_GPIO_NUM       13
#define SIOC_GPIO_NUM       12

#define Y9_GPIO_NUM         39
#define Y8_GPIO_NUM         36
#define Y7_GPIO_NUM         23
#define Y6_GPIO_NUM         18
#define Y5_GPIO_NUM         15
#define Y4_GPIO_NUM         4
#define Y3_GPIO_NUM         14
#define Y2_GPIO_NUM         5

#define VSYNC_GPIO_NUM      27
#define HREF_GPIO_NUM       25
#define PCLK_GPIO_NUM       19

// I2C OLED Screen - SSD1306
#define I2C_SDA             21
#define I2C_SCL             22

// Customizable button
#define BUTTON_1            34

// PIR Sensor
#define AS312_PIN           33

/***************************************
 *  Configuration
 **************************************/
// Uncomment for Software Enabled Access Point instead of connecting to local network
// #define SOFTAP_MODE

// Uncomment to enable PIR detection-based deepsleep wake-up
// #define PIR_SUPPORT_WAKEUP

// When using timed sleep, set the sleep time here
#define uS_TO_S_FACTOR 1000000              /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5                    /* Time ESP32 will go to sleep (in seconds) */

// WiFi access
#include "wifi_config.h"
String macAddress = "GESTURE-CAM-";
String ipAddress = "";

/***************************************
 *  Board initialization
 **************************************/

// Define HTTP server
// Depend ./src/app_httpd.cpp
extern void startCameraServer();

// Instantiate custom button
// Depend BME280 library - https://github.com/mathertel/OneButton
#include <OneButton.h>
OneButton button(BUTTON_1, true);

// Instantiate OLED screen
// Depend OLED library - https://github.com/ThingPulse/esp8266-oled-ssd1306
#include "SSD1306.h"
#include "OLEDDisplayUi.h"
#define SSD1306_ADDRESS 0x3c
SSD1306 oled(SSD1306_ADDRESS, I2C_SDA, I2C_SCL, (OLEDDISPLAY_GEOMETRY)0);
OLEDDisplayUi ui(&oled);

// Used to check I2C connection
bool deviceProbe(uint8_t addr)
{
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

/***************************************
 *  Setup
 **************************************/

bool setupSensor()
{
    pinMode(AS312_PIN, INPUT);
    return true;
}

bool setupDisplay()
{
    static FrameCallback frames[] = {
        [](OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y)
        {
            display->setTextAlignment(TEXT_ALIGN_CENTER);
            display->setFont(ArialMT_Plain_10);

            display->drawString(64 + x, 9 + y, macAddress);
            display->drawString(64 + x, 25 + y, ipAddress);

            if (digitalRead(AS312_PIN)) {
                display->drawString(64 + x, 40 + y, "AS312 Trigger");
            }
        },
        [](OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y)
        {
            display->setTextAlignment(TEXT_ALIGN_CENTER);
            display->setFont(ArialMT_Plain_10);

            display->drawString( 64 + x, 5 + y, "Camera Ready! Use");
            display->drawString(64 + x, 25 + y, "http://" + ipAddress );
            display->drawString(64 + x, 45 + y, "to connect");
        }
    };

    if (!deviceProbe(SSD1306_ADDRESS))return false;
    oled.init();
    oled.setFont(ArialMT_Plain_16);
    oled.setTextAlignment(TEXT_ALIGN_CENTER);
    oled.drawString( oled.getWidth() / 2, oled.getHeight() / 2 - 10, "Gesture Cam");
    oled.display();
    ui.setTargetFPS(30);
    ui.setIndicatorPosition(BOTTOM);
    ui.setIndicatorDirection(LEFT_RIGHT);
    ui.setFrameAnimation(SLIDE_LEFT);
    ui.setFrames(frames, sizeof(frames) / sizeof(frames[0]));
    ui.setTimePerFrame(6000);
    ui.disableIndicator();
    return true;
}

// Broken I2C connection in the design of the board - Cannot connect to the battery manager
// TODO - Requieres soldering on the board if LiPo battery management needed
bool setupPower()
{
#define IP5306_ADDR 0X75
#define IP5306_REG_SYS_CTL0 0x00
    if (!deviceProbe(IP5306_ADDR))return false;
    bool en = true;
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en)
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    else
        Wire.write(0x35); // 0x37 is default reg value
    return Wire.endTransmission() == 0;
}

bool setupCamera()
{
    camera_config_t config;

    // CSI camera configuration - See .\documentation\T_CameraV17_Schematic.pdf
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // Init with high specs to pre-allocate larger buffers
    // TODO - The 8MB PSRAM mounted on the ESP32-WROVER-B is currently not detected - Fix to improve video quality
    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }

    sensor_t *s = esp_camera_sensor_get();
    // Initial sensors are flipped vertically and colors are a bit saturated

    s->set_vflip(s, 1); // Flip sensor vertically (Mounted upside-down)
    s->set_brightness(s, 1); // Slightly increase brightness
    
    // Drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_SVGA);

    return true;
}

void setupNetwork()
{
#ifdef SOFTAP_MODE
    WiFi.mode(WIFI_AP);
    macAddress += WiFi.softAPmacAddress().substring(0, 5);
    WiFi.softAP(macAddress.c_str());
    ipAddress = WiFi.softAPIP().toString();
#else
    Serial.print("Connecting to local network ");
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" WiFi connected");
    ipAddress = WiFi.localIP().toString();
    macAddress += WiFi.macAddress().substring(0, 5);
#endif
}

bool setupButton()
{
    button.attachClick([]() {
        static bool en = false;
        sensor_t *s = esp_camera_sensor_get();
        en = en ? 0 : 1;
        s->set_vflip(s, en);
        if (en) {
            oled.resetOrientation();
        } else {
            oled.flipScreenVertically();
        }
    });

    button.attachDoubleClick([]() {
        if (PWDN_GPIO_NUM > 0) {
            pinMode(PWDN_GPIO_NUM, PULLUP);
            digitalWrite(PWDN_GPIO_NUM, HIGH);
        }

        ui.disableAutoTransition();
        oled.setTextAlignment(TEXT_ALIGN_CENTER);
        oled.setFont(ArialMT_Plain_10);
        oled.clear();

        oled.drawString(oled.getWidth() / 2 - 5, oled.getHeight() / 2 - 20, "Deepsleep");
        oled.drawString(oled.getWidth() / 2, oled.getHeight() / 2 - 10, "Set to be awakened by");
        oled.drawString(oled.getWidth() / 2, oled.getHeight() / 2, "a key press");

        oled.display();
        delay(3000);
        oled.clear();
        oled.displayOff();

#if defined(AS312_PIN) && defined(PIR_SUPPORT_WAKEUP)
        esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << AS312_PIN)), ESP_EXT1_WAKEUP_ANY_HIGH);
#elif defined(BUTTON_1)
        // esp_sleep_enable_ext0_wakeup((gpio_num_t )BUTTON_1, LOW);
        esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_1)), ESP_EXT1_WAKEUP_ALL_LOW);
#else
        esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
#endif
        esp_deep_sleep_start();
    });
    return true;
}

void setup()
{
    Serial.begin(115200);
    Wire.begin(I2C_SDA, I2C_SCL);

    bool status(false);
    Serial.println("Component \tStatus ");
    status = setupDisplay();
    Serial.print("Display   \t"); Serial.println(status);

    status = setupPower();
    Serial.print("Power     \t"); Serial.println(status);

    status = setupSensor();
    Serial.print("MIR sensor\t"); Serial.println(status);

    status = psramFound();
    Serial.print("PSRAM     \t"); Serial.println(status);

    status = setupCamera();
    Serial.print("Camera    \t"); Serial.println(status);

    if (!status) {
        delay(10000); esp_restart();
    }

    status = setupButton();
    Serial.print("Button    \t"); Serial.println(status);

    setupNetwork();

    startCameraServer();

    Serial.print("Camera Ready! Use 'http://");
    Serial.print(ipAddress);
    Serial.println("' to connect");
}

/***************************************
 *  loop
 **************************************/

void loop()
{
    // Update OLED display and button status
    if (ui.update()) {
        button.tick();
    }
}