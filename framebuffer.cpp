#include <iostream>
#include <vector>
#include <cassert>

#include "utils.h"
#include "framebuffer.h"


void FrameBuffer::set_pixel(const size_t x, const size_t y, const uint32_t colour)
{
    assert(img.size() == w*h
                    && x < w
                    && y < h);
    img[x + y*w] = colour;
}


void FrameBuffer::draw_rectangle(const size_t rect_x, const size_t rect_y, 
                                 const size_t rect_w, const size_t rect_h, 
                                 const uint32_t colour)
{
    assert(img.size() == w*h);

    for (size_t i = 0; i < rect_w; i++)
    {
        for (size_t j = 0; j < rect_h; j++)
        {
            size_t cx = rect_x + i;
            size_t cy = rect_y + j;
            
            if (cx < w && cy < h)
                set_pixel(cx, cy, colour);
        }
    }
}


void FrameBuffer::clear(const uint32_t colour)
{
    img = std::vector<uint32_t>(w*h, colour);
}