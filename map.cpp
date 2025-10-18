#include <cassert>

#include "map.h"


static const char map[] = "0000222222220000"\
                          "1              0"\
                          "1      11111   0"\
                          "1     0        0"\
                          "0     0  1110000"\
                          "0     3        0"\
                          "0   10000      0"\
                          "0   3   11100  0"\
                          "5   4   0      0"\
                          "5   4   1  00000"\
                          "0       1      0"\
                          "2       1      0"\
                          "0       0      0"\
                          "0 0000000      0"\
                          "0              0"\
                          "0002222222200000";


Map::Map()
    : m_width(16), m_height(16)
{
    assert(sizeof(map) == m_width*m_height + 1); // +1 for null terminated string
}


size_t Map::width()  const { return m_width; }
size_t Map::height() const { return m_height; }


int Map::get(const size_t i, const size_t j) const
{
    assert(sizeof(map) == m_width*m_height + 1
           && i < m_width 
           && j < m_height);
           
    return map[i + j*m_width] - '0';
}


bool Map::is_empty(const size_t i, const size_t j) const
{
    assert(sizeof(map) == m_width*m_height+1
           && i < m_width 
           && j < m_height);

    return map[i + j*m_width] == ' ';
}