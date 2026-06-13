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

struct vec4
{
    f32 x, y, z, w;
};

union color_rgba
{
    u8 rgba[4];
    struct
    {
        u8 r, g, b, a;
    };
};

static inline f32 Clamp(f32 a, f32 min, f32 max)
{
    if(a <= min) return(min);
    else if(a >= max) return(max);
    else return(a);
}

static inline color_rgba ColorFromVec(vec4 v)
{
    color_rgba result;
    result.r = (u8)(Clamp(v.x, 0.0f, 1.0f) * 255);
    result.g = (u8)(Clamp(v.y, 0.0f, 1.0f) * 255);
    result.b = (u8)(Clamp(v.z, 0.0f, 1.0f) * 255);
    result.a = (u8)(Clamp(v.w, 0.0f, 1.0f) * 255);
    return(result);
}

static inline void WritePixel(u8* image, u32 x, u32 y, u32 stride, color_rgba color)
{
    memcpy(image + (y * stride + x)*4, color.rgba, 4);
}

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
            f32 py = (f32)((f32)y / (f32)image_height);
            f32 px = (f32)((f32)x / (f32)image_width); 
            vec4 color = {1.0f - py, px, py, 255.0f};
            WritePixel(image_data, x, y, image_width, ColorFromVec(color));
        }
    }
    stbi_write_png("out.png", (s32)image_width, (s32)image_height, (s32)bytes_per_pixel, image_data, (s32)(image_width * bytes_per_pixel));

    exit(0);
}