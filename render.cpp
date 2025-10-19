#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <sstream>
#include <iomanip>
#include "SDL.h"

#include "map.h"
#include "utils.h"
#include "player.h"
#include "framebuffer.h"
#include "textures.h"
#include "sprite.h"
#include "render.h"


bool update_player_state(GameState& game_state)
{
    SDL_Event event;
    if (SDL_PollEvent(&event))
    {
        if (SDL_QUIT == event.type || (SDL_KEYDOWN==event.type && SDLK_ESCAPE==event.key.keysym.sym)) return false;
        if (SDL_KEYUP == event.type)
        {
            if ('a' == event.key.keysym.sym || 'd'==event.key.keysym.sym) game_state.player.turn = 0;
            if ('w' == event.key.keysym.sym || 's'==event.key.keysym.sym) game_state.player.walk = 0;
        }
        if (SDL_KEYDOWN==event.type)
        {
            if ('a' == event.key.keysym.sym) game_state.player.turn = -1;
            if ('d' == event.key.keysym.sym) game_state.player.turn =  1;
            if ('w' == event.key.keysym.sym) game_state.player.walk =  1;
            if ('s' == event.key.keysym.sym) game_state.player.walk = -1;
        }
    }

    return true;
}


void update_player_position(GameState& game_state)
{
    game_state.player.direction += double(game_state.player.turn) * 0.05;
    double new_x = game_state.player.x_pos + game_state.player.walk * cos(game_state.player.direction) * 0.05;
    double new_y = game_state.player.y_pos + game_state.player.walk * sin(game_state.player.direction) * 0.05;

    if ((int(new_x) >= 0) && (int(new_x) < int(game_state.map.width())) && (int(new_y) >= 0) && (int(new_y)<int(game_state.map.height())))
    {
        if (game_state.map.is_empty(new_x, game_state.player.y_pos)) game_state.player.x_pos = new_x;
        if (game_state.map.is_empty(game_state.player.x_pos, new_y)) game_state.player.y_pos = new_y;
    }

    // Update distances from the player to each sprite
    for (size_t i = 0; i < game_state.monsters.size(); i++)
    {
        game_state.monsters[i].player_dist = std::sqrt(pow(game_state.player.x_pos - game_state.monsters[i].x_pos, 2) +
                                                        pow(game_state.player.y_pos - game_state.monsters[i].y_pos, 2));
    }
    std::sort(game_state.monsters.begin(), game_state.monsters.end());
}


int wall_x_coord(const double x, const double y, const Texture& texture_walls)
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


void draw_map(FrameBuffer& fb, const std::vector<Sprite> &sprites,
              const Texture &tex_walls, const Map &map,
              const size_t cell_w, const size_t cell_h)
{
    for (size_t j = 0; j < map.height(); j++)
    {
        for (size_t i = 0; i < map.width(); i++)
        {
            if (map.is_empty(i, j)) continue;

            size_t rect_x = i * cell_w;
            size_t rect_y = j * cell_h;
            size_t texture_id = map.get(i, j);
            assert(texture_id < tex_walls.texture_count());

            fb.draw_rectangle(rect_x, rect_y, cell_w, cell_h, tex_walls.get_px_from_texture(0, 0, texture_id)); // Colour taken from  upper left pixel of texture #texture_id
        }
    }

    for (size_t i = 0; i < sprites.size(); i++)
    {
        fb.draw_rectangle(sprites[i].x_pos * cell_w-3, sprites[i].y_pos*cell_h-3, 6, 6, pack_colour(255, 0, 0));
    }
}


void draw_sprite(FrameBuffer& fb, const Sprite& sprite, const std::vector<double>& depth_buffer, const Player& player, const Texture& tex_sprites)
{
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

    for (size_t i = 0; i < sprite_screen_size; i++)
    {
        if (h_offset + int(i) < 0 || h_offset + i >= fb.width()/2) continue;
        if (depth_buffer[h_offset+i] < sprite.player_dist) continue;
        
        for (size_t j = 0; j < sprite_screen_size; j++)
        {
            if (v_offset + int(j) < 0 || v_offset + j >= fb.height()) continue;
            
            uint8_t r, g, b, a;
            uint32_t colour = tex_sprites.get_px_from_texture(i * tex_sprites.texture_size() / sprite_screen_size,
                                                              j * tex_sprites.texture_size() / sprite_screen_size, 
                                                              sprite.texture_id);
            unpack_colour(colour, r, g, b, a);
            if (a>128) fb.set_pixel(fb.width()/2 + h_offset+i, v_offset+j, colour);
        }
    }
}


void render(FrameBuffer& frame_buf, const GameState &game_state)
{
    const Map& map                     = game_state.map;
    const Player& player               = game_state.player;
    const std::vector<Sprite>& sprites = game_state.monsters;
    const Texture& texture_walls       = game_state.texture_walls;
    const Texture& texture_monster     = game_state.texture_monster;

    const size_t frame_buf_w = frame_buf.width();
    const size_t frame_buf_h = frame_buf.height();
    frame_buf.clear(pack_colour(255, 255, 255));

    const size_t cell_w = frame_buf_w/(map.width()*2); // Size of one map cell on the screen
    const size_t cell_h = frame_buf_h/map.height();
    std::vector<double> depth_buffer(frame_buf_w/2, 1e3);

    // Draw FOV and 3D view
    for (size_t i = 0; i < frame_buf_w/2; i++)
    {
        double angle = player.direction-player.fov/2 + player.fov*i/double(frame_buf_w/2);

        // Draw player's line of sight by drawing hypotenuse t until hitting an object
        for (double t = 0; t < 20; t += 0.01)
        {
            double x = player.x_pos + t*cos(angle);
            double y = player.y_pos + t*sin(angle);
            frame_buf.set_pixel(x*cell_w, y*cell_h, pack_colour(190, 190, 190));

            if (map.is_empty(x, y)) continue;

            // Ray touches a wall, so draw the vertical column to create illusion of 3D
            size_t texture_id = map.get(x, y);
            assert(texture_id < texture_walls.texture_count());
            
            double dist = t * cos(angle - player.direction);
            depth_buffer[i] = dist;

            size_t column_height = frame_buf_h/(t*cos(angle - player.direction));
            int texture_x = wall_x_coord(x, y, texture_walls);
            
            const std::vector<uint32_t>& column = texture_walls.get_scaled_column(texture_id, texture_x, column_height);
            int pix_x = i + frame_buf_w/2; // Right half of the screen so +frame_buf.w/2
            
            // Copy the texture column to the framebuffer
            for (size_t j=0; j<column_height; j++) 
            {
                int pix_y = j + frame_buf_h/2 - column_height/2;
                if ((pix_y >= 0) && (pix_y < (int)frame_buf_h)) 
                {
                    frame_buf.set_pixel(pix_x, pix_y, column[j]);
                }
            }
            break;
        }
    }

    draw_map(frame_buf, sprites, texture_walls, map, cell_w, cell_h);

    for (size_t i = 0; i < sprites.size(); i++)
    {
        draw_sprite(frame_buf, sprites[i], depth_buffer, player, texture_monster);
    }
}
