#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>

#define _USE_MATH_DEFINES
#define FOV M_PI/3.0


// Pack into R8G8B8A8 colour
uint32_t pack_colour(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255)
{
    return r + (g<<8) + (b<<16) + (a<<24);
}


void unpack_colour(const uint32_t& colour, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a)
{
    r = (colour >>  0) & 255;
    g = (colour >>  8) & 255;
    b = (colour >> 16) & 255;
    a = (colour >> 24) & 255;
}


void drop_ppm_image(const std::string filename, const std::vector<uint32_t>& img, 
                    const size_t img_w, const size_t img_h)
{
    assert(img.size() == img_w * img_h);

    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << img_w << " " << img_h << "\n255\n";

    for (size_t i=0; i < img_w*img_h; ++i)
    {
        uint8_t r, g, b, a;
        unpack_colour(img[i], r, g, b, a);
        ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
    }

    ofs.close();
}


void draw_rectangle(std::vector<uint32_t>& img, uint32_t colour, 
                    const size_t  img_w, const size_t img_h,
                    const size_t rect_x, const size_t rect_y, 
                    const size_t rect_w, const size_t rect_h)
{
    assert(img.size() == img_w*img_h);

    for (size_t i=0; i<rect_w; i++)
    {
        for (size_t j=0; j<rect_h; j++)
        {
            size_t cx = rect_x+i;
            size_t cy = rect_y+j;
            assert(cx<img_w && cy<img_h);

            img[cx + cy*img_w] = colour;
        }
    }
}


int main()
{
    const size_t img_w = 1024;
    const size_t img_h = 512;

    // Create one-dimensional array to hold our img (row-major), initialised to white
    std::vector<uint32_t> framebuffer(img_w * img_h, pack_colour(255,255,255));

    // Fill frambebuffer with colour gradient
    for (size_t y=0; y < img_h; y++)
    {
        for (size_t x=0; x < img_w; x++)
        {
            uint8_t r = (255*y)/float(img_h);
            uint8_t g = (255*x)/float(img_w);
            uint8_t b = 0;
            framebuffer[x + y*img_w] = pack_colour(r, g, b);
        }
    }

    // Plan view of map
    const size_t map_w = 16;
    const size_t map_h = 16;
    const char map[] = "0000222222220000"\
                       "1              0"\
                       "1      11111   0"\
                       "1     0        0"\
                       "0     0  1110000"\
                       "0     3        0"\
                       "0   10000      0"\
                       "0   0   11100  0"\
                       "0   0   0      0"\
                       "0   0   1  00000"\
                       "0       1      0"\
                       "2       1      0"\
                       "0       0      0"\
                       "0 0000000      0"\
                       "0              0"\
                       "0002222222200000";
    assert(sizeof(map) == map_w*map_h + 1); // +1 for null terminated string

    const size_t rect_w = img_w/map_w;
    const size_t rect_h = img_h/map_h;

    // Draw the map outline
    for (size_t j=0; j<map_h; j++) 
    { 
        for (size_t i=0; i<map_w; i++) 
        {
            if (map[i + j*map_w]==' ') continue; // Skip empty spaces
            size_t rect_x = i*rect_w;
            size_t rect_y = j*rect_h;
            draw_rectangle(framebuffer, pack_colour(0, 255, 255), img_w, img_h, rect_x, rect_y, rect_w, rect_h);
        }
    }

    // Draw player
    float player_x = 3.456F;
    float player_y = 2.345F;
    float player_direction = 1.523F;
    const float fov = FOV;
    draw_rectangle(framebuffer, pack_colour(255, 255, 255), img_w, img_h, player_x*rect_w, player_y*rect_h, 5, 5);

    // Draw FOV
    for (size_t i=0; i<img_w; i++) 
    {
        float angle = player_direction-fov/2 + fov*i/float(img_w);
    
        // Draw player's line of sight by drawing hypotenuse until hitting an object
        for (float hyp=0; hyp<20; hyp+=0.05)
        {
            float cx = player_x + hyp*cos(angle);
            float cy = player_y + hyp*sin(angle);
            if (map[int(cx) + int(cy)*map_w] != ' ') break;  // If we hit a wall/object, break

            size_t pix_x = cx * rect_w;
            size_t pix_y = cy * rect_h;
            framebuffer[pix_x + pix_y*img_w] = pack_colour(255, 255, 255); // Draw white line showing view direction
        }
    }

    drop_ppm_image("./out.ppm", framebuffer, img_w, img_h);

    return 0;
}