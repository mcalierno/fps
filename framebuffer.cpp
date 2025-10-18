#include <iostream>
#include <vector>
#include <cassert>

#include "utils.h"
#include "framebuffer.h"


FrameBuffer::FrameBuffer(size_t width, size_t height, uint32_t colour)
    : m_width(width), m_height(height), m_img(width*height, colour)
{
}


size_t FrameBuffer::width()  const { return m_width; }
size_t FrameBuffer::height() const { return m_height; }
const std::vector<uint32_t>& FrameBuffer::img() const { return m_img; }


void FrameBuffer::set_pixel(const size_t x, const size_t y, const uint32_t colour)
{
    assert(m_img.size() == m_width * m_height
                    && x < m_width
                    && y < m_height);
    m_img[x + y*m_width] = colour;
}


void FrameBuffer::draw_rectangle(const size_t rect_x, const size_t rect_y, 
                                 const size_t rect_w, const size_t rect_h, 
                                 const uint32_t colour)
{
    assert(m_img.size() == m_width * m_height);

    for (size_t i = 0; i < rect_w; i++)
    {
        for (size_t j = 0; j < rect_h; j++)
        {
            size_t cx = rect_x + i;
            size_t cy = rect_y + j;
            
            if (cx < m_width && cy < m_height)
                set_pixel(cx, cy, colour);
        }
    }
}


void FrameBuffer::clear(const uint32_t colour)
{
    m_img = std::vector<uint32_t>(m_width * m_height, colour);
}