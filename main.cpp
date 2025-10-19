#include <iostream>
#include <vector>
#include <SDL.h>
#include <chrono>
#include <thread>

#include "render.h"
#include "utils.h"


bool init(const FrameBuffer& frame_buf, const GameState& game_state, SDL_Window*& window, SDL_Renderer*& renderer, SDL_Texture*& framebuffer_texture)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!game_state.texture_walls.texture_count() || !game_state.texture_monster.texture_count())
    {
        std::cerr << "Failed to load textures" << std::endl;
        return false;
    }

    if (SDL_CreateWindowAndRenderer(frame_buf.width(), frame_buf.height(), SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS, &window, &renderer))
    {
        std::cerr << "Failed to create window and renderer: " << SDL_GetError() << std::endl;
        return false;
    }

    framebuffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, frame_buf.width(), frame_buf.height());
    
    if (!framebuffer_texture)
    {
        std::cerr << "Failed to create framebuffer texture : " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}


int main()
{
    FrameBuffer frame_buf(1024, 512, pack_colour(255, 255, 255));
    GameState  game_state{ Map(),
                           Player{3.456, 2.345, 1.523, M_PI/3., 0, 0},
                           { {3.523, 3.812, 2, 0},  // vector of monster sprites
                             {1.834, 8.765, 0, 0},
                             {5.323, 5.365, 1, 0},
                             {14.32, 13.36, 3, 0},
                             {4.123, 10.76, 1, 0} },
                           Texture("../walltext.bmp", SDL_PIXELFORMAT_ABGR8888), 
                           Texture("../monsters.bmp", SDL_PIXELFORMAT_ABGR8888)};
    
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture*  framebuffer_texture = nullptr;

    if (!init(frame_buf, game_state, window, renderer, framebuffer_texture)) return -1;

    auto t1 = std::chrono::high_resolution_clock::now();
    
    while (true)
    {
        // Sleep if less than 20 ms since last re-rendering
        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
        if (fp_ms.count()<20)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
            continue;
        }
        t1 = t2;

        if (!update_player_state(game_state)) break;
        update_player_position(game_state);
        render(frame_buf, game_state);

        // Copy framebuffer contents to screen
        SDL_UpdateTexture(framebuffer_texture, NULL, reinterpret_cast<const void*>(frame_buf.img().data()), frame_buf.width()*4);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, framebuffer_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(framebuffer_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
