#ifndef MAP_H
#define MAP_H

#include <cstdlib>


class Map
{
    size_t m_width;
    size_t m_height;

public:
    Map();

    size_t width() const;
    size_t height() const;

    int get(const size_t i, const size_t j) const;
    bool is_empty(const size_t i, const size_t j) const;
};


#endif