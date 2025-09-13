#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <sstream>
#include <iomanip>

#include "map.h"
#include "utils.h"
#include "player.h"
#include "framebuffer.h"
#include "textures.h"

int wall_x_coord(const float x, const float y, Texture &texture_walls)
{
    float hit_x = x - floor(x + 0.5);
    float hit_y = y - floor(y + 0.5);
    int texture = hit_x * texture_walls.texture_size;
    
    if (std::abs(hit_y) > std::abs(hit_x)) // Determine whether hit a "vertical" or a "horizontal" wall
        texture = hit_y * texture_walls.texture_size;
    
    if (texture < 0)
        texture += texture_walls.texture_size;
    
    assert((texture >= 0) && (texture < (int)texture_walls.texture_size));
    return texture;
}


void render(FrameBuffer &frame_buf, Map &map, Player &player, Texture &texture_walls)
{
    frame_buf.clear(pack_colour(255, 255, 255));

    const size_t rect_w = frame_buf.w/(map.w*2); // Size of one map cell on the screen
    const size_t rect_h = frame_buf.h/map.h;
    
    // Draw the map
    for (size_t j = 0; j < map.h; j++)
    {
        for (size_t i=0; i<map.w; i++) 
        {
            if (map.is_empty(i, j)) continue;
            size_t rect_x = i*rect_w;
            size_t rect_y = j*rect_h;
            size_t texture_id = map.get(i, j);
            assert(texture_id < texture_walls.texture_count);
            frame_buf.draw_rectangle(rect_x, rect_y, rect_w, rect_h, texture_walls.get_px_from_texture(0, 0, texture_id)); // Colour taken from  upper left pixel of texture #texture_id
        }
    }

    // Draw FOV and 3D view
    for (size_t i = 0; i < frame_buf.w/2; i++)
    {
        float angle = player.direction-player.fov/2 + player.fov*i/float(frame_buf.w/2);

        // Draw player's line of sight by drawing hypotenuse t until hitting an object
        for (float t = 0; t < 20; t += 0.01) 
        {
            float x = player.x_pos + t*cos(angle);
            float y = player.y_pos + t*sin(angle);
            frame_buf.set_pixel(x*rect_w, y*rect_h, pack_colour(160, 160, 160));

            if (map.is_empty(x, y)) continue;

            // Ray touches a wall, so draw the vertical column to create illusion of 3D
            size_t texture_id = map.get(x, y);
            assert(texture_id < texture_walls.texture_count);

            size_t column_height = frame_buf.h/(t*cos(angle - player.direction));
            int texture_x = wall_x_coord(x, y, texture_walls);
            
            std::vector<uint32_t> column = texture_walls.get_scaled_column(texture_id, texture_x, column_height);
            int pix_x = i + frame_buf.w/2; // Right half of the screen so +frame_buf.w/2
            
            // Copy the texture column to the framebuffer
            for (size_t j=0; j<column_height; j++) 
            {
                int pix_y = j + frame_buf.h/2 - column_height/2;
                if ((pix_y >= 0) && (pix_y < (int)frame_buf.h)) 
                {
                    frame_buf.set_pixel(pix_x, pix_y, column[j]);
                }
            }
            break;
        }
    }
}


int main()
{
    FrameBuffer frame_buf{1024, 512, std::vector<uint32_t>(1024*512, pack_colour(255, 255, 255))};
    Player player{3.456, 2.345, 1.523, M_PI/3.0};
    Map map;
    Texture texture_walls("../walltext.png");

    if (!texture_walls.texture_count) 
    {
        std::cerr << "Failed to load wall textures" << std::endl;
        return -1;
    }

    // Draw animation
    for (size_t frame = 0; frame < 360; frame++) 
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << frame << ".ppm";
        player.direction += 2*M_PI/360;

        render(frame_buf, map, player, texture_walls);
        drop_ppm_image(ss.str(), frame_buf.img, frame_buf.w, frame_buf.h);
    }

    return 0;
}