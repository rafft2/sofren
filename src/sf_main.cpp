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
#define MINIMUM(a, b) ((a) < (b) ? (a) : (b))
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

static inline void SfImageWriteToDisk(sf_image *image, const char *filename)
{
    stbi_flip_vertically_on_write(true);
    stbi_write_png(filename, (s32)image->width, (s32)image->height, (s32)image->bytes_per_pixel, image->data, (s32)(image->width * image->bytes_per_pixel));
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

struct sf_vertex
{
    f32 px, py, pz;
};

struct sf_mesh
{
    sf_vertex *vertices;
    u32 vertex_count;
    u32 *indices;
    u32 index_count;
};

#include "float.h"
f32 X_MAX = -FLT_MIN;
f32 X_MIN = FLT_MAX;
f32 Y_MAX = -FLT_MIN;
f32 Y_MIN = FLT_MAX;
static inline void SfMeshMake(sf_mesh *mesh, const char *filename)
{
    FILE *f = fopen(filename, "r");
    char line[128];
    u32 vertex_count = 0;
    u32 index_count = 0;
    while(fgets(line, 128, f) != NULL)
    {
        if(line[0] == 'v')
        {
            vertex_count++;
        }
        else if(line[0] == 'f')
        {
            index_count+=3;
        }
    }
    fseek(f, 0, SEEK_SET);
    sf_vertex *vertices = (sf_vertex*)malloc(vertex_count * sizeof(sf_vertex) + 1);
    u32 *indices = (u32*)malloc(index_count * 3 * sizeof(u32));
    u32 vi = 1;
    u32 fi = 0;
    while(fgets(line, 128, f) != NULL)
    {
        if(line[0] == 'v')
        {
            f32 x, y, z;
            if(sscanf(line + 1, "%f %f %f", &x, &y, &z) != 3)
            {
                printf("error.\n");
            }
            vertices[vi++] = {x, y, z};
            if(x > X_MAX)
            {
                X_MAX = x;
            }
            else if(x < X_MIN)
            {
                X_MIN = x;
            }
            if(y > Y_MAX)
            {
                Y_MAX = y;
            }
            else if(y < Y_MIN)
            {
                Y_MIN = y;
            }
        }
        else if(line[0] == 'f')
        {
            u32 a, b, c;
            if(sscanf(line + 1, "%u %u %u", &a, &b, &c) != 3)
            {
                printf("error.\n");
            }
            indices[fi++] = a;
            indices[fi++] = b;
            indices[fi++] = c;
        }
    }
    fclose(f);

    mesh->vertex_count = vertex_count;
    mesh->index_count = index_count;
    mesh->vertices = vertices;
    mesh->indices = indices;
}

static inline f32 ComputeTriangleArea(vec2 a, vec2 b, vec2 c)
{
    f32 result = 0.5f * ((b.y-a.y)*(b.x+a.x) + (c.y-b.y)*(c.x+b.x) + (a.y-c.y)*(a.x+c.x));
    return(result);
}

static inline void DrawTriangle(sf_image *image, sf_vertex va, sf_vertex vb, sf_vertex vc, color_rgba color)
{
    f32 w = (f32)image->width;
    f32 h = (f32)image->height;
    // super hacky hack:
    // first subtract min so that every value is between 0 and (MAX - MIN)
    // then divide by (MAX - MIN) to get a value between 0 and 1
    // then multiply by width/height to make the vertex fit the entire screen
    vec2 a = {((va.px - X_MIN) / (X_MAX - X_MIN)) * w, ((va.py - Y_MIN) / (Y_MAX - Y_MIN)) * h};
    vec2 b = {((vb.px - X_MIN) / (X_MAX - X_MIN)) * w, ((vb.py - Y_MIN) / (Y_MAX - Y_MIN)) * h};
    vec2 c = {((vc.px - X_MIN) / (X_MAX - X_MIN)) * w, ((vc.py - Y_MIN) / (Y_MAX - Y_MIN)) * h};

#if 0
    // suppose that we have vertices 0->1->2 in increasing y order
    vec2 verts[3] = {a, b, c};
    for(u32 i = 0; i < 3; i++)
    {
        for(u32 j = i; j < 3; j++)
        {
            if(verts[i].y > verts[j].y)
            {
                vec2 tmp = verts[i];
                verts[i] = verts[j];
                verts[j] = tmp;
            }
        }
    }

    vec2 low = verts[0];
    vec2 mid = verts[1];
    vec2 high = verts[2];
    vec2 left = verts[0];
    vec2 right = verts[1];
    if(left.x > right.x)
    {
        left = verts[1];
        right = verts[0];
    }
    u32 low_y = (u32)low.y;
    u32 mid_y = (u32)mid.y;
    u32 high_y = (u32)high.y;

    // 1st half, we need segments from verts[0] to verts[1] and verts[0] to verts[2]
    for(u32 y = low_y; y <= mid_y; y++)
    {
        // 1st line (verts[0] to verts[1])
        f32 m = (verts[1].y - verts[0].y) / (verts[1].x - verts[0].x);
        u32 start_x = (u32)((((f32)y-verts[0].y)/m) + verts[0].x);

        // 2nd line (verts[0] to verts[2])
        m = (verts[2].y - verts[0].y) / (verts[2].x - verts[0].x);
        u32 end_x = (u32)((((f32)y-verts[0].y)/m) + verts[0].x);
        if(start_x < end_x)
        {
            for(u32 x = start_x; x <= end_x; x++)
            {
                if(x < image->width && y < image->height)
                {
                    WritePixel(image, x, y, color);
                }
            }
        }
        else
        {
            for(u32 x = end_x; x <= start_x; x++)
            {
                if(x < image->width && y < image->height)
                {
                    WritePixel(image, x, y, color);
                }
            }
        }
        
    }

    // 2nd half, we need segments from verts[1] to verts[2] and verts[0] to verts[2]
    for(u32 y = mid_y; y <= high_y; y++)
    {
        // 1st line (verts[1] to verts[2])
        f32 m = (verts[2].y - verts[1].y) / (verts[2].x - verts[1].x);
        u32 start_x = (u32)((((f32)y-verts[1].y)/m) + verts[1].x);

        // 2nd line (verts[0] to verts[2])
        m = (verts[2].y - verts[0].y) / (verts[2].x - verts[0].x);
        u32 end_x = (u32)((((f32)y-verts[0].y)/m) + verts[0].x);
        if(start_x < end_x)
        {
            for(u32 x = start_x; x <= end_x; x++)
            {
                if(x < image->width && y < image->height)
                {
                    WritePixel(image, x, y, color);
                }
            }
        }
        else
        {
            for(u32 x = end_x; x <= start_x; x++)
            {
                if(x < image->width && y < image->height)
                {
                    WritePixel(image, x, y, color);
                }
            }
        }
    }
#else
    s32 min_x = (s32)MINIMUM(MINIMUM(a.x, b.x), c.x);
    s32 max_x = (s32)MAXIMUM(MAXIMUM(a.x, b.x), c.x);
    u32 min_y = (u32)MINIMUM(MINIMUM(a.y, b.y), c.y);
    u32 max_y = (u32)MAXIMUM(MAXIMUM(a.y, b.y), c.y);
    f32 total_area = ComputeTriangleArea(a, b, c);
    if(total_area < 1)
    {
        return;
    }

    for(s32 x = min_x; x <= max_x; x++)
    {
        for(u32 y = min_y; y <= max_y; y++)
        {
            vec2 p = {(f32)x, (f32)y};
            f32 alpha = ComputeTriangleArea(p, b, c) / total_area;
            f32 beta = ComputeTriangleArea(p, c, a) / total_area;
            f32 gamma = ComputeTriangleArea(p, a, b) / total_area;
            if(alpha < 0 || beta < 0 || gamma < 0)
            {
                continue;
            }
            else
            {
                WritePixel(image, (u32)x, y, color);
            }
        }
    }
#endif

    color_rgba black = {0, 0, 0, 1};
    DrawLine(image, a, b, black);
    DrawLine(image, b, c, black);
    DrawLine(image, c, a, black);
}

static void SfMeshDraw(sf_image *image, sf_mesh *mesh, color_rgba color)
{
    for(u32 i = 0; i < mesh->index_count; i+=3)
    {
        u32 a = mesh->indices[i]; 
        u32 b = mesh->indices[i + 1];
        u32 c = mesh->indices[i + 2];
        sf_vertex va = mesh->vertices[a];
        sf_vertex vb = mesh->vertices[b];
        sf_vertex vc = mesh->vertices[c];
        DrawTriangle(image, va, vb, vc, color);
    }
}

int main(void)
{
    u32 image_width = 640;
    u32 image_height = 480;
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
    vec4 green = {0.0f, 1.0f, 0.0f, 1.0f};
    DrawLine(image, PixelCenterFromCoords(4, 4), PixelCenterFromCoords(20, 50), ColorFromVec(black));
    DrawRectangle(image, PixelCenterFromCoords(32, 32), PixelCenterFromCoords(56, 10), ColorFromVec(yellow));
    DrawLine(image, PixelCenterFromCoords(32, 32), PixelCenterFromCoords(56, 10), ColorFromVec(red));
   
    sf_mesh *cow = (sf_mesh*)malloc(sizeof(sf_mesh));
    SfMeshMake(cow, "assets/cow.obj");
    SfMeshDraw(image, cow, ColorFromVec(green));

    SfImageWriteToDisk(image, "out.png");

    exit(0);
}