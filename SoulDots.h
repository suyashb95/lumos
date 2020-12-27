#include "FastLED.h"
#include "Timer.h"

enum Behavior {
	STATIC,
	FADE,
	FLASH,
	WAVE,
	BREATHE,
	TWINKLE
};

CRGB *const DEFAULT_COLORS = new CRGB[2] {CRGB::Blue, CRGB::Black};
uint8_t *const DEFAULT_ANCHOR_POINTS = new uint8_t[2] {0, 255};
class SoulDots {

	public:
		SoulDots();

    	SoulDots(const SoulDots& soulDots);

		void set_max_brightness(int brightness);
		void set_animation_rate(int animation_rate);
		void set_behavior(Behavior new_behavior);
		void set_colors(CRGB colors[], uint8_t anchor_points[], int num_colors, int num_anchor_points);

		void begin(
			int num_leds = 0, 
			CRGB* colors = DEFAULT_COLORS, 
			uint8_t* anchor_points = DEFAULT_ANCHOR_POINTS,
      		int num_colors = 2,       
			int num_anchor_points = 2,
			int max_brightness = 50,
			int animation_rate = 50
		);

		void loop();
		void update();

	private:
		CRGBPalette16 _current_palette;
		CRGB* _leds;
		CRGB* _colors;
		uint8_t* _anchor_points;
		int _max_brightness;
		int _animation_rate;
		int _num_leds;
		int _num_colors;
		int _current_task_id;
		Timer _timer;
   		Behavior _behavior;

		CRGBPalette16& create_palette(uint8_t num_colors);
		void generate_uniform_anchor_points(uint8_t);
		uint8_t* generate_offsets();
    	void static_color();
		static void static_color_wrapper(void* soulDots);
		void flash_colors();
		static void flash_colors_wrapper(void* soulDots);
		void fade_colors();
		static void fade_colors_wrapper(void* soulDots);
		void wave_palette();
		static void wave_palette_wrapper(void* soulDots);
		void twinkle_palette();
		static void twinkle_palette_wrapper(void* soulDots);		
};
