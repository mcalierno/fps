#ifndef RENDER_H
#define RENDER_H

#include <vector>

#include "map.h"
#include "player.h"
#include "sprite.h"
#include "framebuffer.h"
#include "textures.h"


struct GameState
{
    Map map;
    Player player;
    std::vector<Sprite> monsters;
    Texture texture_walls;
    Texture texture_monster;
};

bool update_player_state(GameState& game_state);
void update_player_position(GameState& game_state);
void render(FrameBuffer& frame_buf, const GameState& game_state);


#endif