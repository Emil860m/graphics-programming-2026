layout (location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 VertexUV;

out vec3 WorldPos;
out vec2 UV;
out mat3 TBN;
out float WaveHeight;
out vec3 WaveNormal;


uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;
uniform float Time;

uniform float offset;
uniform vec3 CameraPosition;
uniform float tileIndex;


const int NUM_WAVES = 3;

struct Wave
{
    vec2 direction;
    float steepness;
    float wavelength;
    float amplitude;
    float speed;
};

Wave waves[NUM_WAVES] = Wave[](
    Wave(normalize(vec2(0.2, 1.0)), 0.5, 9.0, 0.08, 0.3),
    Wave(normalize(vec2(1.0, 0.7)), 0.4, 6.0, 0.04, 0.4),
    Wave(normalize(vec2(-1.0, 0.3)), 0.3, 3.75, 0.03, 0.5)
);

void main()
{
    vec3 pos = VertexPosition;

    float cameraX = CameraPosition.x;
    float wrappedOffset =
        floor(cameraX / offset) * offset;

    pos.x += wrappedOffset + tileIndex * offset;
    vec3 tangent = vec3(1.0, 0.0, 0.0);
    vec3 binormal = vec3(0.0, 0.0, 1.0);
    
    // Gerstner Waves
    for(int i = 0; i < NUM_WAVES; ++i)
    {
        Wave w = waves[i];

        float k = 2.0 * 3.14159 / w.wavelength; // 2pi
        float c = sqrt(9.81 / k) * w.speed; // gravity

        vec2 d = normalize(w.direction);

        float f = k * (dot(d, pos.xz) - c * Time);

        float a = w.amplitude;
        float q = w.steepness / (k * a * float(NUM_WAVES));

        // Horizontal displacement
        pos.x += q * a * d.x * cos(f);
        pos.z += q * a * d.y * cos(f);

        // Vertical displacement
        pos.y += a * sin(f);

        tangent += vec3(
            -d.x * d.x * (q * a * k * sin(f)),
             d.x * (a * k * cos(f)),
            -d.x * d.y * (q * a * k * sin(f))
        );

        binormal += vec3(
            -d.x * d.y * (q * a * k * sin(f)),
             d.y * (a * k * cos(f)),
            -d.y * d.y * (q * a * k * sin(f))
        );
    }
    
    vec3 normal = normalize(cross(binormal, tangent));

    vec3 T = normalize(mat3(WorldMatrix) * tangent);
    vec3 B = normalize(mat3(WorldMatrix) * binormal);
    vec3 N = normalize(mat3(WorldMatrix) * normal);

    TBN = mat3(T, B, N);

    vec4 worldPos = WorldMatrix * vec4(pos, 1.0);

    WorldPos = worldPos.xyz;
    UV = VertexUV;
    WaveHeight = pos.y;
    WaveNormal = N;

    gl_Position = ViewProjMatrix * worldPos;
}