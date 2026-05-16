layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

out vec3 WorldPos;
out vec3 Normal;
out vec2 UV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float Time;

// Wave settings
uniform float WaveAmplitude = 0.25;
uniform float WaveLength = 6.0;
uniform float WaveSpeed = 1.0;
uniform float Steepness = 0.4;

// Additional wave layers
uniform vec2 WaveDirection1 = normalize(vec2(1.0, 0.3));
uniform vec2 WaveDirection2 = normalize(vec2(0.8, -0.6));
uniform vec2 WaveDirection3 = normalize(vec2(-0.4, 1.0));

const float PI = 3.14159265359;

// Gerstner wave function
vec3 GerstnerWave(
    vec2 direction,
    vec3 position,
    float steepness,
    float wavelength,
    float amplitude,
    float speed,
    inout vec3 tangent,
    inout vec3 binormal
)
{
    float k = 2.0 * PI / wavelength;
    float c = sqrt(9.8 / k) * speed;

    vec2 d = normalize(direction);

    float f = k * (dot(d, position.xz) - c * Time);
    float a = amplitude;

    tangent += vec3(
        -d.x * d.x * (steepness * a * sin(f)),
         d.x * (a * cos(f)),
        -d.x * d.y * (steepness * a * sin(f))
    );

    binormal += vec3(
        -d.x * d.y * (steepness * a * sin(f)),
         d.y * (a * cos(f)),
        -d.y * d.y * (steepness * a * sin(f))
    );

    return vec3(
        d.x * (steepness * a * cos(f)),
        a * sin(f),
        d.y * (steepness * a * cos(f))
    );
}

void main()
{
    vec3 position = aPos;

    vec3 tangent = vec3(1.0, 0.0, 0.0);
    vec3 binormal = vec3(0.0, 0.0, 1.0);

    // Multiple layered waves
    position += GerstnerWave(
        WaveDirection1,
        aPos,
        Steepness,
        WaveLength,
        WaveAmplitude,
        WaveSpeed,
        tangent,
        binormal
    );

    position += GerstnerWave(
        WaveDirection2,
        aPos,
        Steepness * 0.7,
        WaveLength * 0.6,
        WaveAmplitude * 0.5,
        WaveSpeed * 1.3,
        tangent,
        binormal
    );

    position += GerstnerWave(
        WaveDirection3,
        aPos,
        Steepness * 0.5,
        WaveLength * 0.35,
        WaveAmplitude * 0.25,
        WaveSpeed * 1.8,
        tangent,
        binormal
    );

    // Recalculate normal from wave derivatives
    vec3 finalNormal =
        normalize(cross(binormal, tangent));

    // World position
    vec4 world = model * vec4(position, 1.0);

    WorldPos = world.xyz;

    // Correct transformed normal
    Normal =
        normalize(mat3(transpose(inverse(model))) * finalNormal);

    UV = aUV;

    gl_Position =
        projection * view * world;
}
