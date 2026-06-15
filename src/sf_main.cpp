#include "stdio.h"
#include "stdlib.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(push, 0)
#include "stb_image_write.h"
#pragma warning(pop)

typedef unsigned char u8;
typedef signed int s32;
typedef unsigned int u32;
typedef unsigned int b32;
typedef float f32;

#define MAXIMUM(a, b) ((a) > (b) ? (a) : (b))
#define PI32 3.14159265359f
#define EPSILON32 1e-5f

static inline b32 NearZero(f32 value)
{
    return(fabs(value) <= EPSILON32);
}
static inline b32 FloatEquals(f32 a, f32 b)
{
    return(NearZero(a - b));
}

struct vec2
{
    f32 x, y;
};

inline vec2 operator+(vec2 &a, vec2 &b)
{
    vec2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return(result);
}

inline vec2 operator-(vec2 &a, vec2 &b)
{
    vec2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return(result);
}

static inline vec2 PixelCenterFromCoords(u32 x, u32 y)
{
    vec2 result;
    result.x = (f32)x + 0.5f;
    result.y = (f32)y + 0.5f;
    return(result);
}

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

struct sf_image
{
    u8* data;
    u32 width;
    u32 height;
    u32 bytes_per_pixel;
};

sf_image *SfImageMake(u32 width, u32 height)
{
    sf_image *ptr = (sf_image*)malloc(sizeof(sf_image));
    ptr->width = width;
    ptr->height = height;
    ptr->bytes_per_pixel = 4; // NOTE: hardcoded R8B8G8A8 format for PNG
    ptr->data = (u8*)malloc(width * height * ptr->bytes_per_pixel);
    return(ptr); 
}

static inline void WritePixel(sf_image *image, u32 x, u32 y, color_rgba color)
{
    memcpy(image->data + (y * image->width + x)*image->bytes_per_pixel, color.rgba, image->bytes_per_pixel);
}

static inline void DrawLine(sf_image *image, vec2 a, vec2 b, color_rgba color)
{
    f32 dx = fabsf(a.x - b.x);
    f32 dy = fabsf(a.y - b.y);
    f32 step_count = MAXIMUM(dx, dy);
    f32 step = 1.0f / step_count;
    for(f32 t = 0.0f; t <= 1.0f; t += step)
    {
        f32 xt = t * a.x + (1.0f - t) * b.x;
        f32 yt = t * a.y + (1.0f - t) * b.y; 
        WritePixel(image, (u32)xt, (u32)yt, color);
    }
}

static inline void DrawRectangle(sf_image *image, vec2 min_corner, vec2 max_corner, color_rgba color)
{
    vec2 a = min_corner;
    vec2 b = {min_corner.x, max_corner.y};
    vec2 c = max_corner;
    vec2 d = {max_corner.x, min_corner.y};
    DrawLine(image, a, b, color);
    DrawLine(image, b, c, color);
    DrawLine(image, c, d, color);
    DrawLine(image, d, a, color);
}

static inline void SfImageRenderAndWriteToDisk(sf_image *image, const char *filename)
{
    stbi_flip_vertically_on_write(true);
    stbi_write_png(filename, (s32)image->width, (s32)image->height, (s32)image->bytes_per_pixel, image->data, (s32)(image->width * image->bytes_per_pixel));
}

int main(void)
{
    u32 image_width = 64;
    u32 image_height = 64;
    sf_image *image = SfImageMake(image_width, image_height);
    for(u32 y = 0; y < image_height; y++)
    {
        for(u32 x = 0; x < image_width; x++)
        {
            f32 py = (f32)((f32)y / (f32)image_height);
            f32 px = (f32)((f32)x / (f32)image_width); 
            vec4 color = {1.0f - py, px, py, 255.0f};
            WritePixel(image, x, y, ColorFromVec(color));
        }
    }
    vec4 black = {0.0f, 0.0f, 0.0f, 1.0f};
    vec4 red = {1.0f, 0.0f, 0.0f, 1.0f};
    vec4 yellow = {1.0f, 1.0f, 0.0f, 1.0f};
    DrawLine(image, PixelCenterFromCoords(4, 4), PixelCenterFromCoords(20, 50), ColorFromVec(black));
    DrawRectangle(image, PixelCenterFromCoords(32, 32), PixelCenterFromCoords(56, 10), ColorFromVec(yellow));
    DrawLine(image, PixelCenterFromCoords(32, 32), PixelCenterFromCoords(56, 10), ColorFromVec(red));
    SfImageRenderAndWriteToDisk(image, "out.png");

    exit(0);
}