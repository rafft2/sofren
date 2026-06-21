#include "stdio.h"
#include "stdlib.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(push, 0)
#include "stb_image_write.h"
#pragma warning(pop)

#include "sf_math.h"

struct sf_image
{
    u8* data;
    u8* zbuffer;
    u32 width;
    u32 height;
    u32 bytes_per_pixel;

    u8 GetDepth(u32 x, u32 y) { return(zbuffer[y * width + x]); }
};

sf_image *SfImageMake(u32 width, u32 height)
{
    sf_image *ptr = (sf_image*)malloc(sizeof(sf_image));
    ptr->width = width;
    ptr->height = height;
    ptr->bytes_per_pixel = 4; // NOTE: hardcoded R8B8G8A8 format for PNG
    ptr->data = (u8*)malloc(width * height * ptr->bytes_per_pixel);
    ptr->zbuffer = (u8*)malloc(width * height);
    for(u32 i = 0; i < width*height; i++) { ptr->zbuffer[i] = 255; }
    return(ptr); 
}

static inline void SfImageWriteToDisk(sf_image *image, const char *filename)
{
    stbi_flip_vertically_on_write(true);
    s32 ok = stbi_write_png(filename, (s32)image->width, (s32)image->height, (s32)image->bytes_per_pixel, image->data, (s32)(image->width * image->bytes_per_pixel));
    if(!ok)
    {
        printf("error with stbi_write_png.\n");
    }
    else
    {
        printf("wrote image: %s.\n", filename);
    }
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

    vec3 min_dims;
    vec3 max_dims;
};

static inline void SfMeshMake(sf_mesh *mesh, const char *filename)
{
    FILE *f = fopen(filename, "r");
    if(!f)
    {
        printf("cannot open file: %s.\n", filename);
        return;
    }
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
    sf_vertex *vertices = (sf_vertex*)malloc((vertex_count + 1) * sizeof(sf_vertex));
    u32 *indices = (u32*)malloc(index_count * sizeof(u32));
    u32 vi = 1;
    u32 fi = 0;
    f32 x_min = FLT_MAX, y_min = FLT_MAX, z_min = FLT_MAX;
    f32 x_max = FLT_MIN, y_max = FLT_MIN, z_max = FLT_MIN;
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
            
            x_max = MAXIMUM(x_max, x); x_min = MINIMUM(x_min, x);
            y_max = MAXIMUM(y_max, y); y_min = MINIMUM(y_min, y);
            z_max = MAXIMUM(z_max, z); z_min = MINIMUM(z_min, z);
        }
        else if(line[0] == 'f')
        {
            u32 a, b, c;
            if(sscanf(line + 1, "%u %u %u", &a, &b, &c) != 3)
            {
                // TODO: handle format with /
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
    mesh->min_dims = {x_min, y_min, z_min};
    mesh->max_dims = {x_max, y_max, z_max};
}

static inline void WritePixel(sf_image *image, u32 x, u32 y, color_rgba color, u8 depth = 255)
{
    memcpy(image->data + (y * image->width + x)*image->bytes_per_pixel, color.rgba, image->bytes_per_pixel);
    image->zbuffer[y * image->width + x] = depth;
}

static inline void DrawLine(sf_image *image, vec2 a, vec2 b, color_rgba color)
{
    f32 dx = fabsf(a.x - b.x);
    f32 dy = fabsf(a.y - b.y);
    // TODO: shouldn't this be MAXIMUM and then check for ==0?
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

static inline f32 ComputeTriangleArea(vec2 a, vec2 b, vec2 c)
{
    f32 result = 0.5f * ((b.y-a.y)*(b.x+a.x) + (c.y-b.y)*(c.x+b.x) + (a.y-c.y)*(a.x+c.x));
    return(result);
}
static inline f32 ComputeTriangleArea(vec3 a, vec3 b, vec3 c)
{
    f32 result = 0.5f * ((b.y-a.y)*(b.x+a.x) + (c.y-b.y)*(c.x+b.x) + (a.y-c.y)*(a.x+c.x));
    return(result);
}

vec3 UnitCubePosFromVertex(sf_vertex vertex, vec3 min_dims, vec3 max_dims)
{
    vec3 result;
    result.x = (vertex.px - min_dims.x) / (max_dims.x - min_dims.x);
    result.y = (vertex.py - min_dims.y) / (max_dims.y - min_dims.y);
    result.z = (vertex.pz - min_dims.z) / (max_dims.z - min_dims.z);
    return(result);
}
vec3 ScreenPosFromUnitCubePos(vec3 unit_cube_position, f32 screen_width, f32 screen_height)
{
    vec3 result;
    result.x = unit_cube_position.x * (screen_width - 1.0f);
    result.y = unit_cube_position.y * (screen_height - 1.0f);

    // TODO: technically 255 should be the frustum depth
    result.z = unit_cube_position.z * 255.0f;
    return(result);
}

static inline void DrawTriangle(sf_image *image, vec3 min_dims, vec3 max_dims, sf_vertex va, sf_vertex vb, sf_vertex vc)
{
    f32 w = (f32)image->width;
    f32 h = (f32)image->height;

    vec3 unit_cube_a = UnitCubePosFromVertex(va, min_dims, max_dims);
    vec3 unit_cube_b = UnitCubePosFromVertex(vb, min_dims, max_dims);
    vec3 unit_cube_c = UnitCubePosFromVertex(vc, min_dims, max_dims);

    // TODO: should screen positions be vec2?
    vec3 a = ScreenPosFromUnitCubePos(unit_cube_a, w, h);
    vec3 b = ScreenPosFromUnitCubePos(unit_cube_b, w, h);
    vec3 c = ScreenPosFromUnitCubePos(unit_cube_c, w, h);

    f32 total_area = ComputeTriangleArea(a, b, c);
    if(total_area < 1) { return; }
    
    s32 start_x = (s32)MINIMUM(MINIMUM(a.x, b.x), c.x);
    s32 end_x = (s32)MAXIMUM(MAXIMUM(a.x, b.x), c.x);
    s32 start_y = (s32)MINIMUM(MINIMUM(a.y, b.y), c.y);
    s32 end_y = (s32)MAXIMUM(MAXIMUM(a.y, b.y), c.y);
    
    for(s32 x = start_x; x <= end_x; x++)
    {
        for(s32 y = start_y; y <= end_y; y++)
        {
            // TODO: this seems sketchy
            vec3 p = {(f32)x, (f32)y, 255};
            f32 alpha = ComputeTriangleArea(p, b, c) / total_area;
            f32 beta = ComputeTriangleArea(p, c, a) / total_area;
            f32 gamma = ComputeTriangleArea(p, a, b) / total_area;
            if(alpha < 0 || beta < 0 || gamma < 0)
            {
                continue;
            }
            else
            {
                u8 depth = (u8)(alpha * a.z + beta * b.z + gamma * c.z);
                if(depth <= image->GetDepth((u32)x, (u32)y))
                {
                    vec4 color = {alpha, beta, gamma, 1.0f};
                    WritePixel(image, (u32)x, (u32)y, ColorFromVec(color), depth);
                }
            }
        }
    }
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
        // TODO: should DrawTriangle work directly in screen space?
        DrawTriangle(image, mesh->min_dims, mesh->max_dims, va, vb, vc);
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
            vec4 color = {1.0f - py, px, py, 1.0f};
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