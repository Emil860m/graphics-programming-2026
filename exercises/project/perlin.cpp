#include <cmath>
#include <cstdlib>
#include <iostream>
#include <cstdint>
#include <iostream>
#define STB_IMAGE_WRITE_IMPLEMENTATION 
#include "stb_image_write.h"
//using namespace std;

// --------------------------------------------------
// Utility Functions
// --------------------------------------------------

unsigned g_Seed = 0;




static float fade(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

static float randomGradient(int ix, int iy)
{
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;

    unsigned a = ix + g_Seed;
    unsigned b = iy + g_Seed * 31;

    a *= 3284157443;
    b ^= a << s | a >> (w - s);

    b *= 1911520717;
    a ^= b << s | b >> (w - s);

    a *= 2048419325;

    return (a / (float)UINT32_MAX) * 2.0f - 1.0f;
}

static float dotGridGradient(int ix, int iy, float x, float y)
{
    float dx = x - (float)ix;
    float dy = y - (float)iy;

    float angle = randomGradient(ix, iy) * 3.14159265f;

    float gx = cos(angle);
    float gy = sin(angle);

    return dx * gx + dy * gy;
}

// --------------------------------------------------
// Base Perlin Noise
// --------------------------------------------------

float perlin(float x, float y)
{
    int x0 = (int)floor(x);
    int x1 = x0 + 1;

    int y0 = (int)floor(y);
    int y1 = y0 + 1;

    float sx = fade(x - (float)x0);
    float sy = fade(y - (float)y0);

    float n0, n1, ix0, ix1;

    n0 = dotGridGradient(x0, y0, x, y);
    n1 = dotGridGradient(x1, y0, x, y);
    ix0 = lerp(n0, n1, sx);

    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    ix1 = lerp(n0, n1, sx);

    return lerp(ix0, ix1, sy);
}

// --------------------------------------------------
// Tileable Perlin Noise
// repeatX/repeatY define tile period
// --------------------------------------------------

float perlinTiled(
    float x,
    float y,
    int repeatX,
    int repeatY)
{
    int x0 = (int)floor(x);
    int x1 = x0 + 1;

    int y0 = (int)floor(y);
    int y1 = y0 + 1;

    // Wrap lattice points
    x0 = x0 % repeatX;
    x1 = x1 % repeatX;

    y0 = y0 % repeatY;
    y1 = y1 % repeatY;

    if (x0 < 0) x0 += repeatX;
    if (x1 < 0) x1 += repeatX;

    if (y0 < 0) y0 += repeatY;
    if (y1 < 0) y1 += repeatY;

    float sx = fade(x - floor(x));
    float sy = fade(y - floor(y));

    float n0, n1, ix0, ix1;

    n0 = dotGridGradient(x0, y0, x, y);
    n1 = dotGridGradient(x1, y0, x, y);
    ix0 = lerp(n0, n1, sx);

    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    ix1 = lerp(n0, n1, sx);

    return lerp(ix0, ix1, sy);
}


// --------------------------------------------------
// Fractal Brownian Motion (Octaves)
// --------------------------------------------------

float octavePerlin(
    float x,
    float y,
    int octaves,
    float persistence)
{
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;

    float maxValue = 0.0f;

    for (int i = 0; i < octaves; i++)
    {
        total += perlin(x * frequency, y * frequency) * amplitude;

        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return total / maxValue;
}

float octavePerlinTiled(
    float x,
    float y,
    int repeatX,
    int repeatY,
    int octaves)
{
    float total = 0.0f;
    float maxValue = 0.0f;

    float frequency = 1.0f;

    float amplitude = 1.0f;
    for (size_t i = 0; i < octaves; i++)
    {

        total += perlinTiled(
            x * frequency,
            y * frequency,
            (int)(repeatX * frequency),
            (int)(repeatY * frequency)
        ) * amplitude;

        maxValue += amplitude;

        frequency *= 2.0f;
    }

    return total / maxValue;
}


// --------------------------------------------------
// Generate Noise Image
// --------------------------------------------------

unsigned char* generatePerlinNoise(
    int width,
    int height,
    float scale,
    int octaves,
    float persistence,
    float noiseStrength)
{
    unsigned char* data = new unsigned char[width * height];

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float nx = x / scale;
            float ny = y / scale;
            //float noise = -1.0;
            float noise = 
            octavePerlin(
                nx,
                ny,
                octaves,
                persistence);
                /*
                octavePerlinTiled(
                nx,
                ny,
                (int)(width / scale),
                (int)(height / scale),
                octaves
            );
            */
            // Convert from [-1,1] to [0,1]
            noise = (noise + 1.0f) * 0.5f;
            float gradient = (float)y / (float)(height - 1); 
            float finalValue = gradient * (1.0f - noiseStrength) + noise * noiseStrength; 
            //std::cout << noise << "\n";
            // Clamp
            // -------------------------------------- // Blend Noise + Gradient // -------------------------------------- 
            // 0.0 = pure gradient 
            // 1.0 = pure noise 
            //float noiseStrength = 1.0f; 
            // Clamp 
            if (finalValue < 0.0f) finalValue = 0.0f; 
            if (finalValue > 1.0f) finalValue = 1.0f; 
            unsigned char value = (unsigned char)(finalValue * 255.0f);

            data[y * width + x] = value;
        }
    }
    std::cout << strlen((char*)data) << "\n";
    return data;
}

// --------------------------------------------------
// Main
// --------------------------------------------------

int save_image(float noiseStrength, const char* filename, int scale)
{
    g_Seed = (unsigned)time(nullptr);
    const int width = 27 * scale;
    const int height = 15 * scale;

    unsigned char* image =
        generatePerlinNoise(
            width,
            height,
            150.0f, // scale
            8,      // octaves
            0.3f,    // persistence
            noiseStrength
        );

    int success = stbi_write_png(
        filename,
        width,
        height,
        1,
        image,
        width
    );

    if (success)
    {
        std::cout << "Saved perlin_octaves.png\n";
    }
    else
    {
        std::cout << "Failed to save image\n";
    }

    delete[] image;

    return 0;
}


// --------------------------------------------------
// Example Usage
// --------------------------------------------------
/*
int main()
{
    const int width = 256;
    const int height = 256;

    unsigned char* image =
        generatePerlinNoise(width, height, 50.0f);

    // Example: print first few values
    for (int i = 0; i < 10; i++)
    {
        std::cout << (int)image[i] << " ";
    }

    std::cout << std::endl;

    delete[] image;

    return 0;
}
*/
