#include "utils.h"

uint16_t* generate_uniform_anchor_points(uint16_t min, uint16_t max, uint16_t num_anchor_points) {
    uint16_t* anchor_points = new uint16_t[num_anchor_points];
    uint16_t index_step = (max - min) / (num_anchor_points - 1);

    for (uint16_t i = 0; i < num_anchor_points; i++) {
        anchor_points[i] = (i == (num_anchor_points - 1)) ? max - 1 : min + (i * index_step);
    }
    return anchor_points;
} 

uint8_t* generate_random_offsets(uint8_t size) {
    uint8_t *offsets = new uint8_t[size];
    for (uint8_t i = 0; i < size; i++) {
        offsets[i] = random8();
    }
    return offsets;
}