in vec3 WorldPos;
in vec3 Normal;
in vec2 UV;

out vec4 FragColor;

uniform samplerCube Skybox;
uniform sampler2D groundPlane;
uniform sampler2D NormalMap;

uniform vec3 CameraPosition;

uniform float Time;

// Optional tuning uniforms
uniform vec3 DeepWaterColor = vec3(0.0, 0.08, 0.18);
uniform vec3 ShallowWaterColor = vec3(0.0, 0.35, 0.5);

uniform float WaveSpeed = 0.05;
uniform float WaveScale = 0.08;
uniform float ReflectionStrength = 1.0;
uniform float RefractionStrength = 0.03;
uniform float FresnelPower = 5.0;
uniform float SpecularStrength = 1.5;
uniform float Shininess = 128.0;

uniform vec3 LightDirection = normalize(vec3(0.3, 1.0, 0.2));
uniform vec3 LightColor = vec3(1.0);

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
           (c - a) * u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

void main()
{
    vec3 viewDir = normalize(CameraPosition - WorldPos);

    // Animated UVs for waves
    vec2 waveUV1 = UV * 6.0 + vec2(Time * WaveSpeed, Time * 0.03);
    vec2 waveUV2 = UV * 12.0 - vec2(Time * 0.04, Time * 0.06);

    // Procedural wave noise
    float wave1 = noise(waveUV1);
    float wave2 = noise(waveUV2);

    float waveHeight = (wave1 + wave2) * 0.5;

    // Normal map distortion
    vec3 normalTex = texture(NormalMap, waveUV1 + waveUV2).rgb;
    normalTex = normalize(normalTex * 2.0 - 1.0);

    vec3 N = normalize(mix(Normal, normalTex, 0.65));

    // Reflection
    vec3 reflectedDir = reflect(-viewDir, N);
    vec3 reflection = texture(Skybox, reflectedDir).rgb;

    // Refraction / distortion
    vec2 refractUV = UV + N.xz * RefractionStrength;
    vec3 refracted = texture(groundPlane, refractUV).rgb;

    // Fresnel
    float fresnel =
        pow(1.0 - max(dot(viewDir, N), 0.0), FresnelPower);

    // Water depth color
    vec3 waterColor =
        mix(ShallowWaterColor, DeepWaterColor, waveHeight);

    // Combine reflection and refraction
    vec3 color =
        mix(refracted * waterColor,
            reflection,
            fresnel * ReflectionStrength);

    // Specular lighting
    vec3 halfVec =
        normalize(viewDir + normalize(LightDirection));

    float spec =
        pow(max(dot(N, halfVec), 0.0), Shininess);

    vec3 specular =
        LightColor * spec * SpecularStrength;

    // Foam on wave peaks
    float foam =
        smoothstep(0.7, 1.0, waveHeight);

    vec3 foamColor = vec3(1.0) * foam * 0.5;

    // Final color
    color += specular;
    color += foamColor;

    FragColor = vec4(color, 0.92);
}
