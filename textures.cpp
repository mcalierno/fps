#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "utils.h"
#include "textures.h"


Texture::Texture(const std::string& filename)
    : m_img_w(0), m_img_h(0), m_texture_count(0), m_texture_size(0), m_img() 
{
    int w, h;
    int nchannels = -1;
    unsigned char *pixmap = stbi_load(filename.c_str(), &w, &h, &nchannels, 0);
    
    if (!pixmap)
    {
        std::cerr << "Error: can not load the textures" << std::endl;
        return;
    }

    if (nchannels != 4)
    {
        std::cerr << "Error: the texture must be a 32 bit image" << std::endl;
        stbi_image_free(pixmap);
        return;
    }

    if (w != (h * int(w/h)))
    {
        std::cerr << "Error: the texture file must contain N square textures packed horizontally" << std::endl;
        stbi_image_free(pixmap);
        return;
    }

    m_texture_count = w/h;
    m_texture_size = w/m_texture_count;
    m_img_w = w;
    m_img_h = h;

    m_img = std::vector<uint32_t>(w*h);
    for (int j=0; j<h; j++)
    {
         for (int i=0; i<w; i++)
         {
            uint8_t r = pixmap[(i+j*w)*4+0];
            uint8_t g = pixmap[(i+j*w)*4+1];
            uint8_t b = pixmap[(i+j*w)*4+2];
            uint8_t a = pixmap[(i+j*w)*4+3];
            m_img[i+j*w] = pack_colour(r, g, b, a);
        }
    }
    
    stbi_image_free(pixmap);
}


size_t Texture::texture_size() const { return m_texture_size; }
size_t Texture::texture_count() const { return m_texture_count; }


uint32_t Texture::get_px_from_texture(const size_t i, const size_t j, const size_t idx) const
{
    assert(i < m_texture_size && j < m_texture_size && idx < m_texture_count);
    return m_img[i + idx*m_texture_size + j*m_img_w];
}


std::vector<uint32_t> Texture::get_scaled_column(const size_t texture_id, const size_t texture_coord, const size_t column_height) const
{
    assert((texture_coord < m_texture_size) && (texture_id < m_texture_count));
    std::vector<uint32_t> column(column_height);
    
    for (size_t y=0; y<column_height; y++)
    {
        column[y] = get_px_from_texture(texture_coord, (y*m_texture_size)/column_height, texture_id);
    }

    return column;  // automatically uses move
}
