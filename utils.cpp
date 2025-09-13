#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>

#include "utils.h"

// Pack into R8G8B8A8 colour
uint32_t pack_colour(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
    return r + (g<<8) + (b<<16) + (a<<24);
}


void unpack_colour(const uint32_t& colour, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a)
{
    r = (colour >>  0) & 255;
    g = (colour >>  8) & 255;
    b = (colour >> 16) & 255;
    a = (colour >> 24) & 255;
}


void drop_ppm_image(const std::string& filename, const std::vector<uint32_t>& img, 
                    const size_t img_w, const size_t img_h)
{
    assert(img.size() == img_w * img_h);

    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << img_w << " " << img_h << "\n255\n";

    for (size_t i=0; i < img_w*img_h; ++i)
    {
        uint8_t r, g, b, a;
        unpack_colour(img[i], r, g, b, a);
        ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
    }

    ofs.close();
}