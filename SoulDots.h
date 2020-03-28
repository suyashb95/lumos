#include "FastLED.h"
#include "Timer.h"

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
    	SoulDots();
    	SoulDots(const SoulDots& soulDots);

		void set_max_brightness(int brightness);
		void set_animation_rate(int animation_rate);
		void set_behavior(behavior new_behavior);
		void set_colors(CRGB colors[], int num_colors);
		void begin(int num_leds, CRGB* colors = NULL, int num_colors = 2, int animation_rate = 50, int max_brightness = 50);
		void loop();
		void switch_behavior(void* soulDots);

	private:
		CRGBPalette16 _current_palette;
		CRGB* _leds;
		CRGB* _colors;
		int _max_brightness;
		int _animation_rate;
		int _num_leds;
		int _num_colors;
		Timer _timer;
		int _current_task_id;
   		behavior _behavior;

		CRGBPalette16& create_palette();
    	void static_color();
		static void static_color_wrapper(void* soulDots);
		void flash_colors();
		static void flash_colors_wrapper(void* soulDots);
		void fade_colors();
		static void fade_colors_wrapper(void* soulDots);
		void wave_palette();
		static void wave_palette_wrapper(void* soulDots);
};
