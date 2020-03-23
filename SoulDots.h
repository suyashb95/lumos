#include "FastLED.h"

enum behavior {
	STATIC,
	FADE,
	FLASH,
	BREATHE,
	WAVE
};

class SoulDots {

	public:
		SoulDots(int num_leds, CRGB colors[], int num_colors, int max_brightness, int animation_rate);
		SoulDots(int num_leds, CRGB colors[], int num_colors, int max_brightness);
		SoulDots(int num_leds, CRGB colors[], int num_colors);
		SoulDots(int num_leds);

		void set_max_brightness(int brightness);
		void set_animation_rate(int animation_rate);
		void set_behavior(behavior new_behavior);
		void set_colors(CRGB colors[], int num_colors);
		void loop();

	private:
		CRGBPalette16 _current_palette;
		CRGB* _leds = NULL;
		CRGB _colors[2] = { CRGB::Red, CRGB::Blue };
		int _max_brightness;
		int _animation_rate;
		int _num_leds;
		int _num_colors = 2;
		behavior _behavior;

		CRGBPalette16& create_palette();
    	void static_color();
		void flash_colors();
		void fade_colors();
		void wave_palette();
};
