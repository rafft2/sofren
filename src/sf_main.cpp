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

#define MAXIMUM(a, b) ((a) > (b) ? (a) : (b))

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

static inline void WritePixel(u8* image, u32 stride, u32 x, u32 y, color_rgba color)
{
    memcpy(image + (y * stride + x)*4, color.rgba, 4);
}

struct vec2
{
    f32 x, y;
};

static inline vec2 PixelCenterFromCoords(u32 x, u32 y)
{
    vec2 result;
    result.x = (f32)x + 0.5f;
    result.y = (f32)y + 0.5f;
    return(result);
}

static inline void WriteLine(u8 *image, u32 stride, vec2 a, vec2 b, color_rgba color)
{
    f32 dx = fabsf(a.x - b.x);
    f32 dy = fabsf(a.y - b.y);
    f32 step_count = MAXIMUM(dx, dy);
    f32 step = 1.0f / step_count;
    for(f32 t = 0.0f; t <= 1.0f; t += step)
    {
        f32 xt = t * a.x + (1.0f - t) * b.x;
        f32 yt = t * a.y + (1.0f - t) * b.y; 
        WritePixel(image, stride, (u32)xt, (u32)yt, color);
    }
}

int main(void)
{
    u32 image_width = 64;
    u32 image_height = 64;
    u32 bytes_per_pixel = 4;
    u8 *image_data = (u8*)malloc(image_width*image_height*4);
    for(u32 y = 0; y < image_height; y++)
    {
        for(u32 x = 0; x < image_width; x++)
        {
            f32 py = (f32)((f32)y / (f32)image_height);
            f32 px = (f32)((f32)x / (f32)image_width); 
            vec4 color = {1.0f - py, px, py, 255.0f};
            WritePixel(image_data, image_width, x, y, ColorFromVec(color));
        }
    }
    vec4 color = {0.0f, 0.0f, 0.0f, 1.0f};
    vec4 red = {1.0f, 0.0f, 0.0f, 1.0f};
    WriteLine(image_data, image_width, PixelCenterFromCoords(4, 4), PixelCenterFromCoords(20, 50), ColorFromVec(color));
    WriteLine(image_data, image_width, PixelCenterFromCoords(32, 32), PixelCenterFromCoords(56, 10), ColorFromVec(color));
    WriteLine(image_data, image_width, PixelCenterFromCoords(32, 32), PixelCenterFromCoords(56, 10), ColorFromVec(red));
    stbi_flip_vertically_on_write(true);
    stbi_write_png("out.png", (s32)image_width, (s32)image_height, (s32)bytes_per_pixel, image_data, (s32)(image_width * bytes_per_pixel));

    exit(0);
}