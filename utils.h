#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <cstdint>
#include <string>

// Pack into R8G8B8A8 colour
uint32_t pack_colour(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255);

void unpack_colour(const uint32_t& colour, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);

void drop_ppm_image(const std::string& filename, const std::vector<uint32_t>& img, 
                    const size_t img_w, const size_t img_h);

#endif