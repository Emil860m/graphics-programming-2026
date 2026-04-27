in vec3 WorldPos;
in vec3 Normal;

out vec4 FragColor;

uniform samplerCube Skybox;
uniform vec3 CameraPosition;
uniform float Time;

void main()
{
    // --- base vectors ---
    vec3 I = normalize(WorldPos - CameraPosition);
    vec3 N = normalize(Normal);

    // --- simple wave distortion ---
    float wave = sin(WorldPos.x * 0.2 + Time) * 0.1 +
                 cos(WorldPos.z * 0.2 + Time) * 0.1;

    N = normalize(N + vec3(0.0, wave, 0.0));

    // --- refraction ---
    float eta = 1.00 / 1.33; // air → water
    vec3 refrDir = refract(I, N, eta);

    // --- reflection ---
    vec3 reflDir = reflect(I, N);

    vec3 refrColor = texture(Skybox, refrDir).rgb;
    vec3 reflColor = texture(Skybox, reflDir).rgb;

    // --- fresnel (angle-based blend) ---
    float fresnel = pow(1.0 - max(dot(-I, N), 0.0), 5.0);

    vec3 color = mix(refrColor, reflColor, fresnel);

    // slight blue tint
    color = mix(color, vec3(0.0, 0.3, 0.5), 0.2);

    FragColor = vec4(color, 0.9);
}