#include <FastLED.h>
#include <WebSocketsServer.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>


#define NUM_LEDS 60
#define DATA_PIN 19
#define USE_SERIAL Serial

#define COLORS_KEY "colors"
#define BEHAVIOR_KEY "behavior"
#define BRIGHTNESS_KEY "brightness"

enum behaviors {
  STATIC,
  FADE,
  JUMP,
  BREATHE,
  WAVE
};

CRGB leds[NUM_LEDS];
CRGB colors[NUM_LEDS];
CRGBPalette16 palette;

uint8_t i = 0, num_colors=0;
enum behaviors behavior;

WebSocketsServer webSocket = WebSocketsServer(1234);

CRGBPalette16 & createPalette(CRGB colors[]) {
    /*
    creates a gradient palette from the chosen colors(max 16). If less than
    2 colors are chosen, it defaults to a Red -> Black gradient palette

    The colors are used as equally spaced anchor points which form the palette
    Read more here: https://github.com/FastLED/FastLED/wiki/Gradient-color-palettes
    */

    uint8_t num_palette_colors = 2;
    CRGB palette_colors[] = {CRGB::Red, CRGB::Blue};

    if (num_colors >= 2) {
      num_palette_colors = num_colors;
      memset(palette_colors, colors[0], num_colors);
      for (uint8_t i=0; i < num_palette_colors; i++) {
        palette_colors[i] = colors[i];  
      }
    }

    TDynamicRGBGradientPalette_byte palette_anchors[num_palette_colors * 4];

    uint8_t index_step = 256 / (num_palette_colors - 1);

    for(uint8_t i=0; i < num_palette_colors; i++) {
        uint8_t palette_anchor_base = i*4;
        uint8_t color_index = (i == 0) ? 0 : (index_step * i) - 1;

        palette_anchors[palette_anchor_base] = color_index;
        palette_anchors[palette_anchor_base + 1] = palette_colors[i].red;
        palette_anchors[palette_anchor_base + 2] = palette_colors[i].green;
        palette_anchors[palette_anchor_base + 3] = palette_colors[i].blue;
    }
    return palette.loadDynamicGradientPalette(palette_anchors);
}

void switchPatternConfig(DynamicJsonDocument& pattern_config) {
    const char* behaviour = pattern_config[BEHAVIOR_KEY];

    if(!strcmp(behaviour, "STATIC")){
        behavior = STATIC;
    } else if (strcmp(behaviour, "JUMP") == 0) {
        behavior = JUMP;
    } else if (strcmp(behaviour, "FADE") == 0) {
        behavior = FADE;
    } else if (strcmp(behaviour, "WAVE") == 0) {
        behavior = WAVE;
    }
    else {
        behavior = STATIC;
    }
}

void setPalette(CRGB colors[]) {
    palette = createPalette(colors);
}

void setColors(DynamicJsonDocument& pattern_config) {
    JsonArray new_colors = pattern_config["colors"];

    for(uint8_t i=0; i < new_colors.size(); i++) {
        int temp = new_colors[i];
        uint8_t red = (temp >> 16) & 0xFF;
        uint8_t green = (temp >> 8) & 0xFF;
        uint8_t blue = temp & 0xFF;

        colors[i] = CRGB(red, green, blue);
    }
    num_colors = new_colors.size();
}

void setBrightness(DynamicJsonDocument& pattern_config) {
  int brightness = pattern_config[BRIGHTNESS_KEY];
  FastLED.setBrightness(100);
}

void cyclePalette(CRGB leds[], uint8_t num_leds, CRGBPalette16 palette, uint8_t cycle_rate) {
    static uint8_t start_index = 0;
    static uint8_t index_increment = 1; //modify this to control wavelength?

    EVERY_N_MILLISECONDS(cycle_rate) {
        fill_palette(leds, num_leds, start_index, index_increment, palette, 100, LINEARBLEND);
        start_index = (start_index == 255) ? 0 : start_index + 1;
    }
}

void blendColors(CRGB leds[], uint8_t num_leds, CRGB colors[], uint8_t num_colors, uint8_t blend_rate) {
    if (num_colors == 0) return;

    if (num_colors == 1) {
      fill_solid(leds, num_leds, colors[0]);
      return;
    }

    static uint8_t i = 0;
    static uint8_t amount_to_blend = 0;

    static CRGB current_color = colors[0];
    static CRGB target_color = colors[0];
    static CRGB start_color = colors[0];

    EVERY_N_MILLISECONDS(blend_rate) {

        if (target_color == current_color) {
            start_color = colors[i];
            i = (i + 1) % num_colors;
            target_color = colors[i];
            amount_to_blend = 0;
        }

        current_color = blend(start_color, target_color, amount_to_blend);
        fill_solid(leds, num_leds, current_color);
        amount_to_blend++;
    }
}

void jumpColors(CRGB leds[], uint8_t num_leds, CRGB colors[], uint8_t num_colors, uint8_t duration) {
    if (num_colors == 0) return;

    static uint8_t i = 0;
    static CRGB current_color = colors[i];

    EVERY_N_MILLISECONDS(duration) {
        fill_solid(leds, num_leds, current_color);
        current_color = colors[((++i) % num_colors)];
    }
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
                        setColors(patternConfig);
                        setBrightness(patternConfig);
                        setPalette(colors);
                        switchPatternConfig(patternConfig);
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

void handlePatternConfig() {
    switch(behavior) {
        case STATIC: {
            fill_solid(leds, NUM_LEDS, colors[0]);
            break;
        }
        case JUMP: {
            jumpColors(leds, NUM_LEDS, colors, num_colors, 500);
            break;
        }
        case FADE: {
            blendColors(leds, NUM_LEDS, colors, num_colors, 10);
            break;
        }
        case WAVE: {
            cyclePalette(leds, NUM_LEDS, palette, 20);
            break;
        }
        default: {
            fill_solid(leds, NUM_LEDS, colors[0]);
            break;
        }
    }
    FastLED.show();
}

void setup() {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(100);

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

    WiFi.begin("SunCity Surveillance", "P@s$w0rd1234");

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
    handlePatternConfig();
}
