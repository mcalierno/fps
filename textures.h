#include <cstdint>
#include <vector>
#ifndef TEXTURES_H
#define TEXTURES_H


struct Texture 
{
    size_t img_w, img_h;
    size_t texture_count, texture_size;  // number of textures and size in pixels
    std::vector<uint32_t> img; // textures storage container

    Texture(const std::string filename);
    
    // Get the pixel (i,j) from the texture idx
    uint32_t& get_px_from_texture(const size_t px_i, const size_t px_j, const size_t texture_idx); 
    
    // Retrieve one column (tex_coord) from the texture texture_id and scale it to the destination size
    std::vector<uint32_t> get_scaled_column(const size_t texture_id, const size_t texture_coord, const size_t column_height); 
};


#endif