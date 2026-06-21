#ifndef SF_MATH_H

typedef unsigned char u8;
typedef signed int s32;
typedef unsigned int u32;
typedef unsigned int b32;
typedef float f32;

#define MAXIMUM(a, b) ((a) > (b) ? (a) : (b))
#define MINIMUM(a, b) ((a) < (b) ? (a) : (b))
#define PI32 3.14159265359f
#define EPSILON32 1e-5f

#include "float.h"

static inline b32 NearZero(f32 value)
{
    return(fabs(value) <= EPSILON32);
}
static inline b32 FloatEquals(f32 a, f32 b)
{
    return(NearZero(a - b));
}

static inline f32 Clamp(f32 a, f32 min, f32 max)
{
    if(a <= min) return(min);
    else if(a >= max) return(max);
    else return(a);
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

struct vec3
{
    f32 x, y, z;
};

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

static inline color_rgba ColorFromVec(vec4 v)
{
    color_rgba result;
    result.r = (u8)(Clamp(v.x, 0.0f, 1.0f) * 255);
    result.g = (u8)(Clamp(v.y, 0.0f, 1.0f) * 255);
    result.b = (u8)(Clamp(v.z, 0.0f, 1.0f) * 255);
    result.a = (u8)(Clamp(v.w, 0.0f, 1.0f) * 255);
    return(result);
}

#define SF_MATH_H
#endif