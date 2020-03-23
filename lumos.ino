#include "SoulDots.h"
#include <WebSocketsServer.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "config.h"

#define NUM_LEDS 60
#define DATA_PIN 19
#define USE_SERIAL Serial

const char* BEHAVIOR_KEY = "behavior";
const char* BRIGHTNESS_KEY = "brightness";
const char* COLORS_KEY = "colors";
const char* MOTION_RATE_KEY = "motionRate";

WebSocketsServer webSocket = WebSocketsServer(1234);
SoulDots soulDots = SoulDots(60);

void setBehavior(DynamicJsonDocument& pattern_config) {
    const char* behaviour = pattern_config[BEHAVIOR_KEY];
    if(!strcmp(behaviour, "STATIC")){
        soulDots.set_behavior(STATIC);
    } else if (strcmp(behaviour, "JUMP") == 0) {
        soulDots.set_behavior(FLASH);
    } else if (strcmp(behaviour, "FADE") == 0) {
        soulDots.set_behavior(FADE);
    } else if (strcmp(behaviour, "WAVE") == 0) {
        soulDots.set_behavior(WAVE);
    }
    else {
        soulDots.set_behavior(STATIC);
    }
}


void setColors(DynamicJsonDocument& pattern_config) {
    JsonArray new_colors = pattern_config[COLORS_KEY];
    CRGB colors[new_colors.size()];
    for(uint8_t i=0; i < new_colors.size(); i++) {
        int temp = new_colors[i];
        uint8_t red = (temp >> 16) & 0xFF;
        uint8_t green = (temp >> 8) & 0xFF;
        uint8_t blue = temp & 0xFF;

        colors[i] = CRGB(red, green, blue);
    }
    soulDots.set_colors(colors, new_colors.size());
}

void setBrightness(DynamicJsonDocument& pattern_config) {
  soulDots.set_max_brightness(pattern_config[BRIGHTNESS_KEY]);
}

void setMotionRate(DynamicJsonDocument& pattern_config) {
  soulDots.set_animation_rate(pattern_config[MOTION_RATE_KEY]);
}

void updateSoulDots(DynamicJsonDocument& pattern_config) {
    setColors(pattern_config);
    setBrightness(pattern_config);
    setMotionRate(pattern_config);
    setBehavior(pattern_config);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
                IPAddress ip = webSocket.remoteIP(num);
                USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            }
            break;
        case WStype_TEXT: {
                USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
                DynamicJsonDocument patternConfig(1024);
                DeserializationError err = deserializeJson(patternConfig, payload);
                switch(err.code()) {
                    case DeserializationError::Ok:
                        USE_SERIAL.println("Deserialization succeeded");
                        updateSoulDots(patternConfig);
                        break;
                    case DeserializationError::InvalidInput:
                        USE_SERIAL.println("Invalid input!");
                        break;
                    case DeserializationError::NoMemory:
                        USE_SERIAL.println("Not enough memory");
                        break;
                    default:
                        USE_SERIAL.println("Deserialization failed");
                        break;
                }
            }
            break;
    case WStype_ERROR:
            USE_SERIAL.printf("Received error\n");
    case WStype_FRAGMENT_TEXT_START:
            USE_SERIAL.printf("Received text start\n");
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
        case WStype_BIN:
            USE_SERIAL.printf("Received binary\n");
      break;
    }

}

void setup() {
    USE_SERIAL.begin(115200);

    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    WiFi.begin(SSID, PASSWORD);

    while(WiFi.status() != WL_CONNECTED) {
        USE_SERIAL.printf("Attempting connection\n");
        delay(100);
    }

    USE_SERIAL.printf("Wi-Fi connected\n");
    USE_SERIAL.println("Connected!");
    USE_SERIAL.print("My IP address: ");
    USE_SERIAL.println(WiFi.localIP());
    webSocket.onEvent(webSocketEvent);
    webSocket.begin();
    USE_SERIAL.printf("Websocket server started\n");
}

void loop() {
    webSocket.loop();
    soulDots.loop();
}
