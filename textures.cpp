#include <iostream>
#include <vector>
#include <cassert>
#include "SDL.h"

#include "utils.h"
#include "textures.h"


Texture::Texture(const std::string& filename, const uint32_t format)
    : m_img_w(0), m_img_h(0), m_texture_count(0), m_texture_size(0), m_img() 
{
    SDL_Surface* tmp = SDL_LoadBMP(filename.c_str());

    if (!tmp)
    {
        std::cerr << "Error in SDL_LoadBMP: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Surface* surface = SDL_ConvertSurfaceFormat(tmp, format, 0);
    SDL_FreeSurface(tmp);
    
    if (!surface)
    {
        std::cerr << "Error in SDL_ConvertSurfaceFormat: " << SDL_GetError() << std::endl;
        return;
    }

    int w = surface->w;
    int h = surface->h;

    if ((w * 4) != surface->pitch)
    {
        std::cerr << "Error: the texture must be a 32 bit image" << std::endl;
        SDL_FreeSurface(surface);
        return;
    }

    if (w != (h * int(w/h)))
    {
        std::cerr << "Error: the texture file must contain N square textures packed horizontally" << std::endl;
        SDL_FreeSurface(surface);
        return;
    }

    m_texture_count = w/h;
    m_texture_size = w/m_texture_count;
    m_img_w = w;
    m_img_h = h;
    uint8_t* pixmap = reinterpret_cast<uint8_t*>(surface->pixels);

    m_img = std::vector<uint32_t>(w*h);
    for (int j = 0; j < h; j++)
    {
         for (int i = 0; i < w; i++)
         {
            uint8_t r = pixmap[(i + j*w)*4 + 0];
            uint8_t g = pixmap[(i + j*w)*4 + 1];
            uint8_t b = pixmap[(i + j*w)*4 + 2];
            uint8_t a = pixmap[(i + j*w)*4 + 3];
            m_img[i + j*w] = pack_colour(r, g, b, a);
        }
    }
    
    SDL_FreeSurface(surface);
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
    
    for (size_t y = 0; y < column_height; y++)
    {
        column[y] = get_px_from_texture(texture_coord, (y*m_texture_size)/column_height, texture_id);
    }

    return column;  // automatically uses move
}
