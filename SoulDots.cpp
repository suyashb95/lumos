#include "SoulDots.h"
#include "utils.h"

#define DATA_PIN 19
#define MAX_NUM_COLORS 32

SoulDots::SoulDots() {}

SoulDots::SoulDots(const SoulDots& s) {
    _leds = new CRGB[s._num_leds];
    memcpy(_leds, s._leds, s._num_leds * sizeof(CRGB));

    _colors = new CRGB[MAX_NUM_COLORS];
    memcpy(_colors, s._colors, s._num_colors * sizeof(CRGB));

    _anchor_points = new uint16_t[MAX_NUM_COLORS];
    memcpy(_anchor_points, s._anchor_points, s._num_colors * sizeof(uint16_t));

    _num_colors = s._num_colors;
    _num_leds = s._num_leds;
    _animation_rate = s._animation_rate;
    _max_brightness = s._max_brightness;
    _behavior = s._behavior;
}

void SoulDots::begin(
    int num_leds,
    CRGB* colors,
    uint16_t* anchor_points,
    int num_colors,
    int num_anchor_points,
    int max_brightness,
    int animation_rate
    ) {

    _leds = new CRGB[num_leds];
    _num_leds = num_leds;
    _colors = new CRGB[MAX_NUM_COLORS];
    _anchor_points = new uint16_t[MAX_NUM_COLORS];
    _num_colors = num_colors;
    _animation_rate = animation_rate;
    _max_brightness = max_brightness;
    _behavior = STATIC;

    SoulDots::set_colors(colors, anchor_points, num_colors, num_anchor_points);
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(_leds, _num_leds);
    FastLED.setBrightness(_max_brightness);
    SoulDots::update();
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

void SoulDots::set_colors(CRGB* colors, uint16_t* anchor_points, int num_colors, int num_anchor_points) {
    assert(num_colors <= 32 && num_colors >= 1);
    memcpy(_colors, colors, num_colors * sizeof(CRGB));
    _num_colors = num_colors;
    if (anchor_points == NULL || num_anchor_points == 0 || num_colors != num_anchor_points) {
        uint16_t* anchor_points = generate_uniform_anchor_points(0, 256, num_colors);
        memcpy(_anchor_points, anchor_points, num_colors * sizeof(uint16_t));
        delete[] anchor_points;
    } else {
        memcpy(_anchor_points, anchor_points, num_anchor_points * sizeof(uint16_t));
    }
    _current_palette = create_palette(num_colors);
}

CRGBPalette16 &SoulDots::create_palette(uint8_t num_colors) {
    /*
    Use list of colors to generate palette or use CRGB::Black
    as the other anchor in case list has just one color
    */
    uint8_t num_palette_colors = num_colors > 1 ? num_colors : 2;
    TDynamicRGBGradientPalette_byte palette_anchors[num_palette_colors * 4];

    if (_num_colors == 1) {
        palette_anchors[4] = 255;
        palette_anchors[5] = 0;
        palette_anchors[6] = 0;
        palette_anchors[7] = 0;
    }

    for (uint8_t i = 0; i < num_colors; i++) {
        uint8_t palette_anchor_base = i * 4;
        palette_anchors[palette_anchor_base] = _anchor_points[i];
        palette_anchors[palette_anchor_base + 1] = _colors[i].red;
        palette_anchors[palette_anchor_base + 2] = _colors[i].green;
        palette_anchors[palette_anchor_base + 3] = _colors[i].blue;
    }

    return _current_palette.loadDynamicGradientPalette(palette_anchors);
}

void SoulDots::update() {
    _timer.stop(_current_task_id);
    switch (_behavior) {
        case STATIC: {
            _current_task_id = _timer.every(_animation_rate, static_color_wrapper, (void *)this);
            break;
        }
        case FLASH: {
            _current_task_id = _timer.every(_animation_rate, flash_colors_wrapper, (void *)this);
            break;
        }
        case FADE: {
            _current_task_id = _timer.every(_animation_rate, fade_colors_wrapper, (void *)this);
            break;
        }
        case WAVE:{
            _current_task_id = _timer.every(_animation_rate, wave_palette_wrapper, (void *)this);
            break;
        }
        case TWINKLE:{
            _current_task_id = _timer.every(_animation_rate, twinkle_palette_wrapper, (void *)this);
            break;
        }
        default:{
            _current_task_id = _timer.every(_animation_rate, static_color_wrapper, (void *)this);
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

void SoulDots::wave_palette() {
    static uint8_t start_index = 0;
    static uint8_t index_increment = 1;

    fill_palette(_leds, _num_leds, start_index, index_increment, _current_palette, 100, LINEARBLEND);
    start_index = (start_index == 255) ? 0 : start_index + 1;
    FastLED.show();
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

void SoulDots::twinkle_palette() {
    static uint8_t *offsets = generate_random_offsets(_num_leds);
    static uint8_t wave_counter = 0;
    static uint8_t index_offset = 256 / _num_leds;

    for(uint8_t i = 0; i < _num_leds; i++) {
        _leds[i] = blend(
            ColorFromPalette(_current_palette, i * index_offset), CRGB::Black, triwave8(wave_counter + offsets[i])
        );
    }

    wave_counter++;
    FastLED.show();
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

void SoulDots::static_color_wrapper(void* soulDots) {
    SoulDots* thisObject = (SoulDots*)soulDots;
    thisObject->static_color();
}

void SoulDots::wave_palette_wrapper(void* soulDots) {
    SoulDots* thisObject = (SoulDots*) soulDots;
    thisObject->wave_palette();
}

void SoulDots::fade_colors_wrapper(void* soulDots) {
    SoulDots* thisObject = (SoulDots*)soulDots;
    thisObject->fade_colors();
}

void SoulDots::flash_colors_wrapper(void* soulDots) {
    SoulDots* thisObject = (SoulDots*)soulDots;
    thisObject->flash_colors();
}

void SoulDots::twinkle_palette_wrapper(void* soulDots) {
    SoulDots* thisObject = (SoulDots*)soulDots;
    thisObject->twinkle_palette();
}
