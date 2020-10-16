#include "SoulDots.h"

#define DATA_PIN 19

SoulDots::SoulDots(
    int num_leds, 
    CRGB* colors, 
    uint8_t* anchor_points, 
    int num_colors, 
    int num_anchor_points, 
    int max_brightness, 
    int animation_rate
    ) {

    _leds = new CRGB[num_leds];
    _num_leds = num_leds;

    if (colors == NULL)  {
        _colors = new CRGB[2] {CRGB::Red, CRGB::Blue};
        _anchor_points = new uint8_t[2] {0, 125};
        _num_colors = 2;
    } else {
        memcpy(_colors, colors, num_colors * sizeof(CRGB));
        if (anchor_points == NULL || num_colors != num_anchor_points) {
            memcpy(_anchor_points, generate_uniform_anchor_points(num_colors), num_colors * sizeof(uint8_t));
        } else {
            memcpy(_anchor_points, anchor_points, num_colors * sizeof(uint8_t));
        }
        _num_colors = num_colors;
    }

    _animation_rate = animation_rate;
    _max_brightness = max_brightness;
    _behavior = STATIC;
}

SoulDots::SoulDots(const SoulDots& s) {
    _leds = new CRGB[s._num_leds];
    memcpy(_leds, s._leds, s._num_leds * sizeof(CRGB));

    _colors = new CRGB[s._num_colors];
    memcpy(_colors, s._colors, s._num_colors * sizeof(CRGB));

    _anchor_points = new uint8_t[s._num_colors];
    memcpy(_anchor_points, s._anchor_points, s._num_colors * sizeof(uint8_t));

    _num_colors = s._num_colors;
    _num_leds = s._num_leds;
    _animation_rate = s._animation_rate;
    _max_brightness = s._max_brightness;
    _behavior = s._behavior;
}

void SoulDots::begin(
    int num_leds, 
    CRGB* colors, 
    uint8_t* anchor_points, 
    int num_colors, 
    int num_anchor_points, 
    int max_brightness, 
    int animation_rate
    ) {

    /*
    second initialization method because the stack, heap and registers etc aren't set up before the setup() function
    is called so even though the constructor is called in global scope, the members might not have been initialized
    */
    _leds = new CRGB[num_leds];
    _num_leds = num_leds;

    if (colors == NULL)  {
        _colors = new CRGB[2] {CRGB::Red, CRGB::Blue};
        _anchor_points = new uint8_t[2] {0, 125};
        _num_colors = 2;
    } else {
        memcpy(_colors, colors, num_colors * sizeof(CRGB));
        if (anchor_points == NULL || num_colors != num_anchor_points) {
            memcpy(_anchor_points, generate_uniform_anchor_points(num_colors), num_colors * sizeof(uint8_t));
        } else {
            memcpy(_anchor_points, anchor_points, num_colors * sizeof(uint8_t));
        }        
        _num_colors = num_colors;
    }

    _animation_rate = animation_rate;
    _max_brightness = max_brightness;
    _behavior = STATIC;

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

void SoulDots::set_behavior(Behavior new_behavior) {
    _behavior = new_behavior;
}

void SoulDots::set_colors(CRGB* colors, uint8_t* anchor_points, int num_colors, int num_anchor_points) {
    if (num_colors >= 2) {
        memcpy(_colors, colors, num_colors * sizeof(CRGB));
        if (anchor_points == NULL || num_colors != num_anchor_points) {
            memcpy(_anchor_points, generate_uniform_anchor_points(num_colors), num_colors * sizeof(uint8_t));
        } else {
            memcpy(_anchor_points, anchor_points, num_colors * sizeof(uint8_t));
        } 
        _num_colors = num_colors;
        _current_palette = create_palette();
    }
}

CRGBPalette16 &SoulDots::create_palette() {
    TDynamicRGBGradientPalette_byte palette_anchors[_num_colors * 4];

    uint8_t index_step = 256 / (_num_colors - 1);

    for (uint8_t i = 0; i < _num_colors; i++) {
        uint8_t palette_anchor_base = i * 4;
        palette_anchors[palette_anchor_base] = _anchor_points[i];
        palette_anchors[palette_anchor_base + 1] = _colors[i].red;
        palette_anchors[palette_anchor_base + 2] = _colors[i].green;
        palette_anchors[palette_anchor_base + 3] = _colors[i].blue;
    }
    return _current_palette.loadDynamicGradientPalette(palette_anchors);
}

void SoulDots::switch_behavior(void* soulDots) {
    _timer.stop(_current_task_id);
    switch (_behavior) {
        case STATIC: {
            _current_task_id = _timer.every(_animation_rate, static_color_wrapper, soulDots);
            break;
        }
        case FLASH: {
            _current_task_id = _timer.every(_animation_rate, flash_colors_wrapper, soulDots);
            break;
        }
        case FADE: {
            _current_task_id = _timer.every(_animation_rate, fade_colors_wrapper, soulDots);
            break;
        }
        case WAVE:{
            _current_task_id = _timer.every(_animation_rate, wave_palette_wrapper, soulDots);
            break;
        }
        default:{
            _current_task_id = _timer.every(_animation_rate, static_color_wrapper, soulDots);
            break;
        }
    }
}

void SoulDots::loop() {
  _timer.update();
}

void SoulDots::static_color() {
    fill_solid(_leds, _num_leds, _colors[0]);
    FastLED.show();
}

void SoulDots::static_color_wrapper(void* soulDots) {
    SoulDots* thisObject = (SoulDots*)soulDots;
    thisObject->static_color();
}

void SoulDots::wave_palette() {
    static uint8_t start_index = 0;
    static uint8_t index_increment = 1;

    fill_palette(_leds, _num_leds, start_index, index_increment, _current_palette, 100, LINEARBLEND);
    start_index = (start_index == 255) ? 0 : start_index + 1;
    FastLED.show();
}

void SoulDots::wave_palette_wrapper(void* soulDots) {
    SoulDots* thisObject = (SoulDots*) soulDots;
    thisObject->wave_palette();
}

void SoulDots::flash_colors() {
    if (_num_colors == 0)
        return;

    if (_num_colors == 1) {
        static_color();
        return;
    }

    static uint8_t i = 0;
    static CRGB current_color = _colors[i];

    fill_solid(_leds, _num_leds, current_color);
    current_color = _colors[((++i) % _num_colors)];
    FastLED.show();
}

void SoulDots::flash_colors_wrapper(void* soulDots) {
    SoulDots* thisObject = (SoulDots*)soulDots;
    thisObject->flash_colors();
}

void SoulDots::fade_colors() {
    if (_num_colors == 0)
        return;

    if (_num_colors == 1) {
        static_color();
        return;
    }

    static uint8_t i = 0;
    static uint8_t amount_to_blend = 0;

    static CRGB current_color = _colors[0];
    static CRGB target_color = _colors[0];
    static CRGB start_color = _colors[0];

    if (target_color == current_color) {
        start_color = _colors[i];
        i = (i + 1) % _num_colors;
        target_color = _colors[i];
        amount_to_blend = 0;
    }

    current_color = blend(start_color, target_color, amount_to_blend);
    fill_solid(_leds, _num_leds, current_color);
    amount_to_blend++;
    FastLED.show();
}

void SoulDots::fade_colors_wrapper(void* soulDots) {
    SoulDots* thisObject = (SoulDots*)soulDots;
    thisObject->fade_colors();
}

uint8_t* SoulDots::generate_uniform_anchor_points(int num_anchor_points) {
    uint8_t anchor_points[num_anchor_points];
    uint8_t index_step = 256 / (num_anchor_points - 1);

    for (uint8_t i = 0; i < num_anchor_points; i++) {
        uint8_t anchor_point = (i == 0) ? 0 : (index_step * i) - 1;
        anchor_point = (i == (num_anchor_points - 1)) ? 255 : anchor_point;
        anchor_points[i] = anchor_point;
    }
    return anchor_points;  
}
