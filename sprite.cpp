#include "sprite.h"

bool Sprite::operator<(const Sprite& sprite) const {
      return player_dist > sprite.player_dist;
}