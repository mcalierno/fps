#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <cstdint>
#include <vector>


class FrameBuffer
{
    size_t m_width;
    size_t m_height;
    std::vector<uint32_t> m_img;

public:
    // Constructor initialises whole m_img to one colour
    FrameBuffer(size_t width, size_t height, uint32_t colour);

    size_t width() const;
    size_t height() const;
    const std::vector<uint32_t>& img() const;

    void set_pixel(const size_t x, const size_t y, 
                   const uint32_t colour);
    
    void draw_rectangle(const size_t x, const size_t y, 
                        const size_t w, const size_t h, 
                        const uint32_t colour);
    
    void clear(const uint32_t colour);
};


#endif