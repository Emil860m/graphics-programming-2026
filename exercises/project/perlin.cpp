#include <cmath>
#include <cstdlib>
#include <iostream>
#include <cstdint>
#include <iostream>
//using namespace std;

// --------------------------------------------------
// Utility Functions
// --------------------------------------------------

static float lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

static float fade(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

static float randomGradient(int ix, int iy)
{
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;

    unsigned a = ix;
    unsigned b = iy;

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

// --------------------------------------------------
// Generate Noise Image
// --------------------------------------------------

unsigned char* generatePerlinNoise(
    int width,
    int height,
    float scale,
    int octaves,
    float persistence,
    float gradient_strength)
{
    unsigned char* data = new unsigned char[width * height];

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float nx = x / scale;
            float ny = y / scale;

            float noise = octavePerlin(
                nx,
                ny,
                octaves,
                persistence);

            // Convert from [-1,1] to [0,1]
            noise = (noise + 1.0f) * 0.5f;
            //std::cout << noise << "\n";
            // Clamp
            float gradient = (float)x / (float)(width - 1); 
            // -------------------------------------- // Blend Noise + Gradient // -------------------------------------- 
            // 0.0 = pure gradient 
            // 1.0 = pure noise 
            float noiseStrength = gradient_strength; 
            float finalValue = gradient * (1.0f - noiseStrength) + noise * noiseStrength; 
            // Clamp 
            if (finalValue < 0.0f) finalValue = 0.0f; 
            if (finalValue > 1.0f) finalValue = 1.0f; 
            unsigned char value = (unsigned char)(finalValue * 255.0f);

            data[y * width + x] = value;
        }
    }

    return data;
}

// --------------------------------------------------
// Main
// --------------------------------------------------
/*
int main()
{
    const int width = 1024;
    const int height = 1024;

    unsigned char* image =
        generatePerlinNoise(
            width,
            height,
            150.0f, // scale
            6,      // octaves
            0.5f    // persistence
        );

    int success = stbi_write_png(
        "perlin_octaves.png",
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
