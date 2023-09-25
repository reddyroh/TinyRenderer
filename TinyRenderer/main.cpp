#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

const int width = 800, height = 800;

void line(TGAImage& image, int x0, int y0, int x1, int y1, const TGAColor& color);
void triangle(TGAImage& image, Vec3f* points, float* zbuffer, TGAImage& texture, Vec2f* tex_coords, float intensity);
Vec3f barycentric(Vec3f PA, Vec3f AB, Vec3f AC);

int main(int argc, char** argv)
{
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage texture;
    texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.flip_vertically();

    Model* model = new Model("obj/african_head.obj");

    // Define light direction
    Vec3f light_dir(0, 0, -1);

    // Initialize z-buffer to negative infinity
    float* zbuffer = new float[width * height];
    for (int i = 0; i < width * height; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    for (int f = 0; f < model->nfaces(); f++) {
        Vec3i face = model->face(f);
        Vec3f screen_coords[3];
        Vec3f world_coords[3];

        Vec3i facetex = model->faceTex(f);
        Vec2f tex_coords[3];

        for (int v = 0; v < 3; v++) {
            // The vertex data is in Normalized Device Coordinates, range [-1, 1]. 
            // Add 1 to shift this range to [0, 2]. Then multiple by width and height for x and y respectively.
            // Now x is in range [0, 2 * width] and y is in range [0, 2 * height]. Divide by 2.
            // Now x is in [0, width], y is in [0, height]. The coordinates are in screen space. 

            world_coords[v] = model->vert(face.raw[v]);
           
            int x = (world_coords[v].x + 1.0) * width / 2.0;
            int y = (world_coords[v].y + 1.0) * height / 2.0;
            int z = world_coords[v].z;
            screen_coords[v] = Vec3f(x, y, z);

            // Load texture coordinates
            Vec3f vertTex = model->vertTex(facetex.raw[v]);
            tex_coords[v] = Vec2f(vertTex.x, vertTex.y);
        }

        // Compute normal vector as cross product of two triangle sides
        Vec3f normal = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
        normal.normalize();

        // Compute lighting intensity as dot product of normal vector and light direction vector
        float intensity = normal * light_dir;

        // If dot product is negative the light is coming from behind the triangle, we discard it (back-face culling).
        // If dot product is positive, we draw the triangle with appropriate lighting.
        if (intensity > 0) {
            triangle(image, screen_coords, zbuffer, texture, tex_coords, intensity);
        }
    }

    // Want to have the origin at the bottom left corner of the image instead of the top left corner.
    image.flip_vertically();

    image.write_tga_file("output.tga"); 
    // delete model;
    return 0;
}

// Bresenham's line drawing algorithm
void line(TGAImage& image, int x0, int y0, int x1, int y1, const TGAColor& color)
{
    bool steep = false;
    if (abs(y0-y1) > abs(x0-x1)) {
        steep = true;
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    // Ensure point 0 is to the left of point 1
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = 2 * abs(dy);
    int error2 = 0;
    int y = y0;
    int sy = (y0 < y1) ? 1 : -1;

    if (steep) {
        for (int x = x0; x <= x1; x++)
        {
            image.set(y, x, color);
            error2 += derror2;
            if (error2 > dx) {
                y += sy;
                error2 -= 2 * dx;
            }
        }
    }
    else {
        for (int x = x0; x <= x1; x++)
        {
            image.set(x, y, color);
            error2 += derror2;
            if (error2 > dx) {
                y += sy;
                error2 -= 2 * dx;
            }
        }
    }
}

Vec3f barycentric(Vec3f PA, Vec3f AB, Vec3f AC) {
    Vec3f cross = Vec3f(PA.x, AB.x, AC.x) ^ Vec3f(PA.y, AB.y, AC.y);
    
    // Degenerate triangle case
    if (abs(cross.x) < 1) {
        return Vec3f(-1, 1, 1);
    }

    float u = cross.y / cross.x;
    float v = cross.z / cross.x;
    return Vec3f(1 - u - v, u, v);
}

void triangle(TGAImage& image, Vec3f* points, float* zbuffer, TGAImage& texture, Vec2f* tex_coords, float intensity) {
    int tex_width = texture.get_width(), tex_height = texture.get_height();

    Vec2f bboxMin(image.get_width() - 1, image.get_height() - 1);
    Vec2f bboxMax(0, 0);
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);

    for (int i = 0; i < 3; i++) {
        bboxMin.x = std::max(0.0f, std::min(bboxMin.x, points[i].x));
        bboxMin.y = std::max(0.0f, std::min(bboxMin.y, points[i].y));
        
        bboxMax.x = std::min(clamp.x, std::max(bboxMax.x, points[i].x));
        bboxMax.y = std::min(clamp.y, std::max(bboxMax.y, points[i].y));
    }

    Vec3f p;

    for (p.x = bboxMin.x; p.x <= bboxMax.x; p.x++) {
        for (p.y = bboxMin.y; p.y <= bboxMax.y; p.y++) {
            Vec3f bary = barycentric(points[0] - p, points[1] - points[0], points[2] - points[0]);
            if (bary.x < 0 || bary.y < 0 || bary.z < 0) {
                continue;
            }

            // use barycentric coordinates to compute z-value
            Vec3f pts_z = Vec3f(points[0].z, points[1].z, points[2].z);
            p.z = bary * pts_z;

            int idx = static_cast<int>(p.x + p.y * width);
            if (zbuffer[idx] < p.z) {
                zbuffer[idx] = p.z;

                // Interpolate to find exact texture coordinates for this point
                Vec2f coords = Vec2f(0, 0);
                for (int i = 0; i < 3; i++) {
                    coords = coords + tex_coords[i] * bary.raw[i];
                }

                // Get color from the diffuse texture, apply light intensity, set the pixel
                TGAColor color = texture.get(coords.x * tex_width, coords.y * tex_height);
                TGAColor final_color = TGAColor(color.r * intensity, color.g * intensity, color.b * intensity, 255);
                image.set(p.x, p.y, final_color);
            }   
        }
    }
}

void triangle0(TGAImage& image, Vec2i v0, Vec2i v1, Vec2i v2, const TGAColor& color)
{
    // Sort vectors by y-value ascending
    if (v0.y > v2.y) std::swap(v0, v2);
    if (v0.y > v1.y) std::swap(v0, v1);
    if (v1.y > v2.y) std::swap(v1, v2);

    int total_height = v2.y - v0.y;

    // draw bottom half triangle
    for (int y = v0.y; y <= v1.y; y++) {
        int segment_height = v1.y - v0.y + 1; // +1 to avoid divide by zero
        float alpha = (y - v0.y) / static_cast<float>(total_height);
        float beta = (y - v0.y) / static_cast<float>(segment_height);
        Vec2i A = v0 + (v2 - v0) * alpha;
        Vec2i B = v0 + (v1 - v0) * beta;
        
        if (A.x > B.x) std::swap(A, B);
        for (int j = A.x; j <= B.x; j++) {
            image.set(j, y, color);
        }
    }

    // draw top half triangle
    for (int y = v1.y; y <= v2.y; y++) {
        int segment_height = v2.y - v1.y + 1; // +1 to avoid divide by zero
        float alpha = (y - v0.y) / static_cast<float>(total_height);
        float beta = (y - v1.y) / static_cast<float>(segment_height);
        Vec2i A = v0 + (v2 - v0) * alpha;
        Vec2i B = v1 + (v2 - v1) * beta;

        if (A.x > B.x) std::swap(A, B);
        for (int j = A.x; j <= B.x; j++) {
            image.set(j, y, color);
        }
    }
}
