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
#include "sprite.h"


int wall_x_coord(const double x, const double y, Texture &texture_walls)
{
    double hit_x = x - floor(x + 0.5);
    double hit_y = y - floor(y + 0.5);
    int texture = hit_x * texture_walls.texture_size();
    
    if (std::abs(hit_y) > std::abs(hit_x)) // Determine whether hit a "vertical" or a "horizontal" wall
        texture = hit_y * texture_walls.texture_size();
    
    if (texture < 0)
        texture += texture_walls.texture_size();
    
    assert((texture >= 0) && (texture < (int)texture_walls.texture_size()));
    return texture;
}


void show_sprite_on_map(Sprite& sprite, FrameBuffer& fb, Map& map)
{
    const size_t rect_w = fb.width()/(map.width()*2);
    const size_t rect_h = fb.height()/map.height();

    fb.draw_rectangle(sprite.x_pos * rect_w - 3,
                      sprite.y_pos * rect_h - 3,
                      6, 6, pack_colour(255, 0,0));  // FIXME tidy up
}


void draw_sprite(Sprite& sprite, std::vector<double>& depth_buffer, FrameBuffer& fb, Player& player, Texture& tex_sprites) {
    double sprite_direction = atan2(sprite.y_pos - player.y_pos, sprite.x_pos - player.x_pos);
    
    while (sprite_direction - player.direction >  M_PI)
    {
        sprite_direction -= 2*M_PI;
    }
    while (sprite_direction - player.direction < -M_PI)
    {
        sprite_direction += 2*M_PI;
    }

    size_t sprite_screen_size = std::min(1000, static_cast<int>(fb.height()/sprite.player_dist));
    int h_offset = (sprite_direction - player.direction) / player.fov * (fb.width()/2)
                 + (fb.width()/2)/2 - tex_sprites.texture_size()/2;
    int v_offset = fb.height()/2 - sprite_screen_size/2;

    for (size_t i=0; i<sprite_screen_size; i++)
    {
        if (h_offset + int(i) < 0 || h_offset + i >= fb.width()/2) continue;
        if (depth_buffer[h_offset+i] < sprite.player_dist) continue;
        
        for (size_t j=0; j<sprite_screen_size; j++)
        {
            if (v_offset + int(j) < 0 || v_offset + j >= fb.height()) continue;
            
            uint8_t r,g,b,a;
            uint32_t colour = tex_sprites.get_px_from_texture(i * tex_sprites.texture_size() / sprite_screen_size,
                                                              j * tex_sprites.texture_size() / sprite_screen_size, 
                                                              sprite.texture_id);
            unpack_colour(colour, r, g, b, a);
            if (a>128) fb.set_pixel(fb.width()/2 + h_offset+i, v_offset+j, colour);
        }
    }
}


void render(FrameBuffer& frame_buf, Map& map, Player& player, std::vector<Sprite>& sprites, Texture& texture_walls, Texture& texture_monster)
{
    frame_buf.clear(pack_colour(255, 255, 255));

    const size_t rect_w = frame_buf.width()/(map.width()*2); // Size of one map cell on the screen
    const size_t rect_h = frame_buf.height()/map.height();
    
    // Draw the map
    for (size_t j = 0; j < map.height(); j++)
    {
        for (size_t i=0; i<map.width(); i++) 
        {
            if (map.is_empty(i, j)) continue;
            size_t rect_x = i*rect_w;
            size_t rect_y = j*rect_h;
            size_t texture_id = map.get(i, j);
            assert(texture_id < texture_walls.texture_count());
            frame_buf.draw_rectangle(rect_x, rect_y, rect_w, rect_h, texture_walls.get_px_from_texture(0, 0, texture_id)); // Colour taken from  upper left pixel of texture #texture_id
        }
    }

    // Draw FOV and 3D view
    std::vector<double> depth_buffer(frame_buf.width()/2, 1e3);
    for (size_t i = 0; i < frame_buf.width()/2; i++)
    {
        double angle = player.direction-player.fov/2 + player.fov*i/double(frame_buf.width()/2);

        // Draw player's line of sight by drawing hypotenuse t until hitting an object
        for (double t = 0; t < 20; t += 0.01) 
        {
            double x = player.x_pos + t*cos(angle);
            double y = player.y_pos + t*sin(angle);
            frame_buf.set_pixel(x*rect_w, y*rect_h, pack_colour(160, 160, 160));

            if (map.is_empty(x, y)) continue;

            // Ray touches a wall, so draw the vertical column to create illusion of 3D
            size_t texture_id = map.get(x, y);
            assert(texture_id < texture_walls.texture_count());
            
            double dist = t * cos(angle - player.direction);
            depth_buffer[i] = dist;

            size_t column_height = frame_buf.height()/(t*cos(angle - player.direction));
            int texture_x = wall_x_coord(x, y, texture_walls);
            
            const std::vector<uint32_t>& column = texture_walls.get_scaled_column(texture_id, texture_x, column_height);
            int pix_x = i + frame_buf.width()/2; // Right half of the screen so +frame_buf.w/2
            
            // Copy the texture column to the framebuffer
            for (size_t j=0; j<column_height; j++) 
            {
                int pix_y = j + frame_buf.height()/2 - column_height/2;
                if ((pix_y >= 0) && (pix_y < (int)frame_buf.height())) 
                {
                    frame_buf.set_pixel(pix_x, pix_y, column[j]);
                }
            }
            break;
        }
    }

    // Update the distances from the player to each sprite
    for (size_t i = 0; i < sprites.size(); i++)
    { 
        sprites[i].player_dist = std::sqrt(pow(player.x_pos - sprites[i].x_pos, 2) + pow(player.y_pos - sprites[i].y_pos, 2));
    }
    std::sort(sprites.begin(), sprites.end());

    // Draw sprites
    for (size_t i=0; i<sprites.size(); i++)
    {
        show_sprite_on_map(sprites[i], frame_buf, map);
        draw_sprite(sprites[i], depth_buffer, frame_buf, player, texture_monster);
    }
}


int main()
{
    FrameBuffer frame_buf(1024, 512, pack_colour(255, 255, 255));
    Player player{3.456, 2.345, 1.523, M_PI/3.0};
    Map map;
    Texture texture_walls("./walltext.png");
    Texture texture_monster("./monsters.png");

    if (!texture_walls.texture_count()) 
    {
        std::cerr << "Failed to load wall textures" << std::endl;
        return -1;
    }

    std::vector<Sprite> sprites{ {3.523, 3.812, 2, 0}, {1.834, 8.765, 0, 0}, {5.323, 5.365, 1, 0}, {4.123, 10.265, 1, 0} };

    // Draw animation
    render(frame_buf, map, player, sprites, texture_walls, texture_monster);
    drop_ppm_image("./out.ppm", frame_buf.img(), frame_buf.width(), frame_buf.height());
    
    return 0;
}