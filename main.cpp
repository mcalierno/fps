#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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


bool load_texture(const std::string& filename, std::vector<uint32_t>& texture, size_t& text_size, size_t& text_cnt)
{
    int w, h;
    int nchannels = -1;
    
    unsigned char* pixmap = stbi_load(filename.c_str(), &w, &h, &nchannels, 0);

    if (!pixmap)
    {
        std::cerr << "Error: can not load the textures" << std::endl;
        return false;
    }

    if (nchannels != 4)
    {
        std::cerr << "Error: the texture must be a 32 bit image" << std::endl;
        stbi_image_free(pixmap);
        return false;
    }

    texture = std::vector<uint32_t>(w*h);

    for (int j = 0; j < h; j++)
    {
        for int (i = 0; i < w; i++)
        {
            uint8_t r = pixmap[(i + j*w)*4 + 0];
            uint8_t g = pixmap[(i + j*w)*4 + 1];
            uint8_t b = pixmap[(i + j*w)*4 + 2];
            uint8_t a = pixmap[(i + j*w)*4 + 3];
            texture[i+j*w] = pack_colour(r, g, b, a);
        }
    }

    stbi_image_free(pixmap);
    return true;
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
            if (cx>=img_w || cy>=img_h) continue;

            img[cx + cy*img_w] = colour;
        }
    }
}


int main()
{
    const size_t img_w = 1024;
    const size_t img_h = 512;

    float player_x = 3.456F;
    float player_y = 2.345F;
    float player_direction = 1.523F;
    const float fov = FOV;

    const size_t num_colours = 10;
    std::vector<uint32_t> colours(num_colours);
    for (size_t i = 0; i < num_colours; i++)
    {
        colours[i] = pack_colour(rand()%255, rand()%255, rand()%255);
    }

    size_t walltext_size;  // texture dimensions (square)
    size_t walltext_cnt;   // number of different textures in the image
    if (!load_texture("../walltext.png", walltext, walltext_size, walltext_cnt)) 
    {
        std::cerr << "Failed to load wall textures" << std::endl;
        return -1;
    }

    // Create one-dimensional array to hold our img (row-major), initialised to white
    std::vector<uint32_t> framebuffer(img_w * img_h, pack_colour(255,255,255));

    // Fill frambebuffer with colour gradient
    // for (size_t y=0; y < img_h; y++)
    // {
    //     for (size_t x=0; x < img_w; x++)
    //     {
    //         uint8_t r = (255*y)/float(img_h);
    //         uint8_t g = (255*x)/float(img_w);
    //         uint8_t b = 0;
    //         framebuffer[x + y*img_w] = pack_colour(r, g, b);
    //     }
    // }

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

    const size_t rect_w = img_w/(map_w*2);
    const size_t rect_h = img_h/map_h;

    // Draw animation
    for (size_t frame = 0; frame < 360; frame++)
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << frame << ".ppm";
        player_direction += 2*M_PI/360;

        framebuffer = std::vector<uint32_t>(img_w*img_h, pack_colour(255, 255, 255)); // clear the screen

        // Draw the map outline
        for (size_t j=0; j<map_h; j++) 
        { 
            for (size_t i=0; i<map_w; i++) 
            {
                if (map[i + j*map_w] == ' ') continue; // Skip empty spaces
                size_t rect_x = i*rect_w;
                size_t rect_y = j*rect_h;
                size_t icolor = map[i + j*map_w] - '0';
                assert(icolor < num_colours);
                draw_rectangle(framebuffer, pack_colour(0, 255, 255), img_w, img_h, rect_x, rect_y, rect_w, rect_h);
            }
        }

        // Draw player
        draw_rectangle(framebuffer, pack_colour(255, 255, 255), img_w, img_h, player_x*rect_w, player_y*rect_h, 5, 5);

        // Draw FOV and 3D view
        for (size_t i=0; i<img_w/2; i++) 
        {
            float angle = player_direction-fov/2 + fov*i/float(img_w/2);
        
            // Draw player's line of sight by drawing hypotenuse until hitting an object
            for (float hyp=0; hyp<20; hyp+=0.01)
            {
                float cx = player_x + hyp*cos(angle);
                float cy = player_y + hyp*sin(angle);
                // if (map[int(cx) + int(cy)*map_w] != ' ') break;  // If we hit a wall/object, break

                size_t pix_x = cx * rect_w;
                size_t pix_y = cy * rect_h;
                framebuffer[pix_x + pix_y*img_w] = pack_colour(160, 160, 160); // Draw white line showing view direction

                // Ray touches a wall, so draw the vertical column to create illusion of 3D
                if (map[int(cx) + int(cy)*map_w] != ' ') 
                {
                    size_t icolour = map[int(cx) + int(cy)*map_w] - '0';
                    assert(icolour < num_colours);

                    size_t column_height = img_h/(hyp * cos(angle-player_direction)); // Correct fisheye distortion
                    draw_rectangle(framebuffer, colours[icolour], img_w, img_h, img_w/2+i, img_h/2-column_height/2, 1, column_height);
                    break;
                }
            }
        }

        drop_ppm_image(ss.str(), framebuffer, img_w, img_h);
    }


    return 0;
}