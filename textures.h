#include <cstdint>
#include <vector>
#ifndef TEXTURES_H
#define TEXTURES_H


class Texture 
{
    size_t m_img_w;
    size_t m_img_h;

    size_t m_texture_count;
    size_t m_texture_size;  // In pixels

    std::vector<uint32_t> m_img;  // Textures storage container

public:
    Texture(const std::string& filename, const uint32_t format);

    size_t texture_size() const;
    size_t texture_count() const;
    
    // Get the pixel (i,j) from the texture idx
    uint32_t get_px_from_texture(const size_t px_i, const size_t px_j, const size_t texture_idx) const; 
    
    // Retrieve one column (tex_coord) from the texture texture_id and scale it to the destination size
    std::vector<uint32_t> get_scaled_column(const size_t texture_id, const size_t texture_coord, const size_t column_height) const; 
};


#endif