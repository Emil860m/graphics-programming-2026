
#include <glm/vec2.hpp>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <map>
#include "stb_image_write.h"
#include <cstdint>
#include <iostream>
#include <ctime>
#include <utility>
#include <deque>




unsigned Seed = 0;

float _smoothstep(float t) {
    return t * t * (3.0f - 2.0 * t);
}


float _lerp(float t, float a, float b) {
    return a + t * (b - a);
}

glm::vec2 generateGradient() {
    float angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159265f;
    return glm::vec2(cos(angle), sin(angle));

}


float getInterpolatedNoise(std::pair<float, float> coords, std::map<std::pair<int, int>, glm::vec2>& gradients, int tileSize) {
    std::vector<std::pair<int, int>> gridCoords; 

    int minX = (int)floor(coords.first) % (tileSize - 1);
    int maxX = (int)(minX + 1) % (tileSize - 1);
    gridCoords.push_back(std::pair(minX, maxX));
    int minY = (int)floor(coords.second) % (tileSize - 1);
    int maxY = (int)(minY + 1) % (tileSize - 1);
    gridCoords.push_back(std::pair(minY, maxY));
    std::deque<float> dots;
    for (int xi : {minX, maxX})
    {
        for (int yi : {minY, maxY}) {
            glm::vec2 gradient = gradients[std::pair(xi, yi)];
    
            float dot = gradient[0] * (coords.first - xi) + gradient[1] * (coords.second - yi);
    
            dots.push_back(dot);
        }
    }

    int dim = 2;
    while (dots.size() > 1) {
        dim -= 1;
        float s;
        if (dim == 1)
            s = _smoothstep(coords.second - gridCoords[dim].first);
        else if (dim == 0)
        {
            s = _smoothstep(coords.first - gridCoords[dim].first);
        }
        

        std::deque<float> next_dots;
        while (!dots.empty())
        {
            float d1 = dots.front();
            dots.pop_front();
            float d2 = dots.front();
            dots.pop_front();
            next_dots.push_back(_lerp(s, d1, d2));
        }
        dots = next_dots;
    }
    return dots[0];
}


float* generate_perlin_noise_octave(int height, int width, int gradient_grid_size, float amplitude, int tile_size, float gradient_strength) {
    std::map<std::pair<int, int>, glm::vec2> localGradients;
    for (size_t i = 0; i < gradient_grid_size; i++)
    {
        for (size_t j = 0; j < gradient_grid_size; j++)
        {
            localGradients[std::pair(i, j)] = generateGradient();
        }
    }

    for (size_t i = 0; i < gradient_grid_size; i++)
    {
        localGradients[std::pair(i, gradient_grid_size - 1)] = localGradients[std::pair(i, 0)];
    }
    
    for (size_t i = 0; i < gradient_grid_size; i++)
    {
        localGradients[std::pair(gradient_grid_size - 1, i)] = localGradients[std::pair(0, i)];
    }
    
    float* data = new float[height * width];
    float min = 100000.0f;
    float max = -100000.0f;
    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
            float noise = getInterpolatedNoise(
                std::pair(((float)i / height) * (gradient_grid_size - 1), ((float)j / width) * (gradient_grid_size - 1)),
                localGradients,
                tile_size
            );
            float grad = (float) i / (height - 1);
            noise = noise * (1 - gradient_strength) + grad * gradient_strength;
            //noise = (((noise + 1.0f) * 0.5f) * 255.0f);
            //std::cout << noise << "\n";
            data[i * width + j] = noise; 
            if (noise > max) max = noise;
            else if (noise < min) min = noise;
            
            
        }
    }
    for (int i = 0; i < height * width; i++)
    {
        data[i] = ((float)data[i] - min) / (max - min);
        data[i] *= amplitude;
    }
    return data;

}

void save_image(const char* filename)
{
    Seed = (unsigned)time(nullptr);
    const int width = 256;
    const int height = 256;

    unsigned char* image = (unsigned char*)generate_perlin_noise_octave(width, height, 5, 1.0f, 6, 0.5f);
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
}