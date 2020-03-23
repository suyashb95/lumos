#include "SoulDots.h"

#define DATA_PIN 19

SoulDots::SoulDots(int num_leds) {
    _leds = new CRGB[num_leds];
    _num_leds = num_leds;
    _animation_rate = 50;
    _max_brightness = 100;
    _behavior = STATIC;
    _current_palette = create_palette();
        
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(_leds, _num_leds);
    FastLED.setBrightness(_max_brightness);
}

SoulDots::SoulDots(int num_leds, CRGB colors[], int num_colors) {
    _leds = new CRGB[num_leds];
    _num_leds = num_leds;
    memcpy(_colors, colors, num_colors * sizeof(CRGB));
    _num_colors = num_colors;
    _animation_rate = 50;
    _max_brightness = 100;
    _behavior = STATIC;
    _current_palette = create_palette();

    FastLED.addLeds<NEOPIXEL, DATA_PIN>(_leds, _num_leds);
    FastLED.setBrightness(_max_brightness);
}

SoulDots::SoulDots(int num_leds, CRGB colors[], int num_colors, int max_brightness) {
    _leds = new CRGB[num_leds];
    _num_leds = num_leds;
    memcpy(_colors, colors, num_colors * sizeof(CRGB));
    _num_colors = num_colors;
    _animation_rate = 50;
    _max_brightness = max_brightness;
    _behavior = STATIC;
    _current_palette = create_palette();

    FastLED.addLeds<NEOPIXEL, DATA_PIN>(_leds, _num_leds);
    FastLED.setBrightness(_max_brightness);
}

SoulDots::SoulDots(int num_leds, CRGB colors[], int num_colors, int max_brightness, int animation_rate) {
    _leds = new CRGB[num_leds];
    _num_leds = num_leds;
    memcpy(_colors, colors, num_colors * sizeof(CRGB));
    _num_colors = num_leds;
    _animation_rate = animation_rate;
    _max_brightness = max_brightness;
    _behavior = STATIC;
    _current_palette = create_palette();

    FastLED.addLeds<NEOPIXEL, DATA_PIN>(_leds, _num_leds);
    FastLED.setBrightness(_max_brightness);
}

void SoulDots::set_max_brightness(int max_brightness) {
    _max_brightness = max_brightness;
    FastLED.setBrightness(_max_brightness);
}

void SoulDots::set_animation_rate(int animation_rate) {
    _animation_rate = animation_rate;
}

void SoulDots::set_behavior(behavior new_behavior) {
    _behavior = new_behavior;
}

void SoulDots::set_colors(CRGB colors[], int num_colors) {
    if (num_colors < 2) {
        CRGB temp[2] = { CRGB::Red, CRGB::Blue };
        memcpy(_colors, temp, 2 * sizeof(CRGB));
    } else {
        memcpy(_colors, colors, num_colors * sizeof(CRGB));
        _num_colors = num_colors;
    }
    _current_palette = create_palette();
}

CRGBPalette16& SoulDots::create_palette() {
    TDynamicRGBGradientPalette_byte palette_anchors[_num_colors * 4];

    uint8_t index_step = 256 / (_num_colors-1);

    for(uint8_t i=0; i < _num_colors; i++) {
        uint8_t palette_anchor_base = i*4;
        uint8_t color_index = (i == 0) ? 0 : (index_step * i) - 1;
        color_index = (i == (_num_colors - 1)) ? 255 : color_index;
        palette_anchors[palette_anchor_base] = color_index;
        palette_anchors[palette_anchor_base + 1] = _colors[i].red;
        palette_anchors[palette_anchor_base + 2] = _colors[i].green;
        palette_anchors[palette_anchor_base + 3] = _colors[i].blue;
    }
    return _current_palette.loadDynamicGradientPalette(palette_anchors);
}

void SoulDots::loop() {
    switch(_behavior) {
        case STATIC: {
            static_color();
            break;
        }
        case FLASH: {
            flash_colors();
            break;
        }
        case FADE: {
            fade_colors();
            break;
        }
        case WAVE: {
            wave_palette();
            break;
        }
        default: {
            static_color();
            break;
        }
    }
    FastLED.show();
}

void SoulDots::static_color() {
    fill_solid(_leds, _num_leds, _colors[0]);
}

void SoulDots::wave_palette() {
    static uint8_t start_index = 0;
    static uint8_t index_increment = 1;

    EVERY_N_MILLISECONDS(_animation_rate) {
        fill_palette(_leds, _num_leds, start_index, index_increment, _current_palette, 100, LINEARBLEND);
        start_index = (start_index == 255) ? 0 : start_index + 1;
    }
}

void SoulDots::flash_colors() {
    if (_num_colors == 0) return;

    if (_num_colors == 1) {
        static_color();
        return;
    }

    static uint8_t i = 0;
    static CRGB current_color = _colors[i];

    EVERY_N_MILLISECONDS(_animation_rate) {
        fill_solid(_leds, _num_leds, current_color);
        current_color = _colors[((++i) % _num_colors)];
    }
}

void SoulDots::fade_colors() {
    if (_num_colors == 0) return;

    if (_num_colors == 1) {
        static_color();
        return;
    }

    static uint8_t i = 0;
    static uint8_t amount_to_blend = 0;

    static CRGB current_color = _colors[0];
    static CRGB target_color = _colors[0];
    static CRGB start_color = _colors[0];

    EVERY_N_MILLISECONDS(_animation_rate) {

        if (target_color == current_color) {
            start_color = _colors[i];
            i = (i + 1) % _num_colors;
            target_color = _colors[i];
            amount_to_blend = 0;
        }

        current_color = blend(start_color, target_color, amount_to_blend);
        fill_solid(_leds, _num_leds, current_color);
        amount_to_blend++;
    }
}
