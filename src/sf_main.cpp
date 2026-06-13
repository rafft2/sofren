#include "stdio.h"
#include "stdlib.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(push, 0)
#include "stb_image_write.h"
#pragma warning(pop)

typedef unsigned char u8;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;

int main(void)
{
    u32 image_width = 640;
    u32 image_height = 480;
    u32 bytes_per_pixel = 4;
    u8 *image_data = (u8*)malloc(image_width*image_height*4);
    for(u32 y = 0; y < image_height; y++)
    {
        for(u32 x = 0; x < image_width; x++)
        {
            u8* pixel = image_data + (y * image_width + x)*4;
            f32 py = (f32)((f32)y / (f32)image_height);
            f32 px = (f32)((f32)x / (f32)image_width); 
            pixel[0] = (u8)((1.0f - py) * 255);
            pixel[1] = (u8)(px * 255);
            pixel[2] = (u8)(py * 255);
            pixel[3] = 255;
        }
    }
    stbi_write_png("out.png", (s32)image_width, (s32)image_height, (s32)bytes_per_pixel, image_data, (s32)(image_width * bytes_per_pixel));

    printf("Hello, Weird Gradient!");
    exit(0);
}