in vec3 WorldPos;
in vec3 Normal;
in vec2 UV;

out vec4 FragColor;

uniform vec4 source_color;

uniform samplerCube Skybox;
uniform sampler2D groundPlane;
uniform sampler2D NormalMap;

uniform vec3 CameraPosition;
uniform float Time;

/*
void main()
{
    // --- animate UVs ---
    vec2 uv1 = UV * 0.05 + vec2(Time * 0.05, 0.0);
    vec2 uv2 = UV * 0.05 + vec2(0.0, Time * 0.03);

    // --- sample normal map (2 layers = richer waves) ---
    vec3 n1 = texture(NormalMap, uv1).rgb;
    vec3 n2 = texture(NormalMap, uv2).rgb;

    vec3 normalTex = normalize(n1 + n2);

    // convert from [0,1] → [-1,1]
    normalTex = normalTex * 2.0 - 1.0;

    // --- build TBN (simple since plane is flat) ---
    vec3 N = normalize(Normal);
    vec3 T = normalize(vec3(1.0, 0.0, 0.0));
    vec3 B = cross(N, T);

    mat3 TBN = mat3(T, B, N);

    // --- transform to world space ---
    vec3 finalNormal = normalize(normalTex);

    // --- view direction ---
    vec3 I = normalize(WorldPos - CameraPosition);

    // --- refraction ---
    float eta = 1.0 / 1.33;
    vec3 refrDir = refract(I, finalNormal, eta);

    // --- reflection ---
    vec3 reflDir = reflect(I, finalNormal);

    vec3 refrColor = texture(groundPlane, refrDir.xy).rgb;
    vec3 reflColor = texture(Skybox, reflDir).rgb;

    // --- fresnel ---
    float fresnel = pow(1.0 - max(dot(-I, finalNormal), 0.0), 5.0);

    vec3 color = mix(refrColor, reflColor, fresnel);

    // slight water tint
    color = mix(color, source_color.rgb, 0.2);

    FragColor = vec4(color, 0.9);
}
*/

void main()
{    
    vec2 uv1 = UV * 0.05 + vec2(Time * 0.05, 0.0);
    vec2 uv2 = UV * 0.05 + vec2(0.0, Time * 0.03);

    vec3 n1 = texture(NormalMap, uv1).rgb;
    vec3 n2 = texture(NormalMap, uv2).rgb;

    vec3 normalTex = normalize(n1 + n2);
    
    vec3 I = normalize(WorldPos - CameraPosition);
    
        // --- refraction ---
    float eta = 1.0 / 1.33;
    vec3 refrDir = refract(I, normalize(Normal), eta);

    // --- reflection ---
    vec3 reflDir = reflect(I, normalize(Normal));
    float fresnel = pow(1.0 - max(dot(-I, normalize(Normal)), 0.0), 5.0);
    //vec2 refrUV = UV + refrDir.xy;
    vec3 refrColor = texture(groundPlane, normalTex.xy).rgb;
    //vec3 refrColor = texture(Skybox, refrDir).rgb;
    vec3 reflColor = texture(Skybox, reflDir).rgb;
    vec3 color = mix(refrColor, reflColor, fresnel);

    // slight water tint
    color = mix(color, source_color.rgb, 0.3);
    //FragColor = texture(groundPlane, UV);
    FragColor = vec4(color, 0.9);
}
