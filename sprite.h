#ifndef SPRITE_H
#define SPRITE_H

#include <cstdlib>


struct Sprite
{
    double x_pos;
    double y_pos;
    size_t texture_id;
    double player_dist;

    bool operator<(const Sprite& sprite) const;
};


#endif