in vec3 WorldPos;
in vec2 UV;
in mat3 TBN;
in float WaveHeight;
in vec3 WaveNormal;

out vec4 FragColor;

uniform vec4 source_color;

uniform samplerCube Skybox;
uniform sampler2D groundPlane;
uniform sampler2D Normal1;
uniform sampler2D Normal2;

uniform float normalMap1Str;
uniform float normalMap2Str;

uniform float uv_scale;
uniform float movement_strength;
uniform vec2 movement_direction;
uniform vec2 texelSize;
uniform float distortion_strength;
uniform float fresnel_amount;
uniform float foam_amount;

uniform vec3 CameraPosition;
uniform float Time;


float fresnel(float amount, vec3 normal, vec3 view) {
    return pow(1.0 - max(dot(view, normal), 0.0), amount);
}

vec3 getNormal(sampler2D tex, vec2 uv) {
    float hL = texture(tex, uv - vec2(texelSize.x, 0)).r;
    float hR = texture(tex, uv + vec2(texelSize.x, 0)).r;
    float hD = texture(tex, uv - vec2(0, texelSize.y)).r;
    float hU = texture(tex, uv + vec2(0, texelSize.y)).r;
    vec3 normalTS = normalize(vec3(
        hL - hR,
        hD - hU,
        1.0
    ));
    return normalTS;
}


void main()
{  
    vec2 uv = UV * uv_scale;
    uv = fract(uv);
    vec2 uv_movement = movement_direction * Time * movement_strength;
    vec3 n1 = getNormal(Normal1, uv + uv_movement);
    vec3 n2 = getNormal(Normal2, uv - uv_movement);
    // combine
    vec3 normalMap = normalize(n1 + n2);
    vec3 normalMapTBN = normalize(TBN * normalMap);

    // Viewspace
    vec3 I = normalize(WorldPos - CameraPosition);


    //refraction
    float eta = 1.0 / 1.33;
    vec3 refrDir = refract(I, normalMapTBN, eta);
    vec2 refrUV = UV + normalMap.xy * distortion_strength;
    vec3 refrColor = texture(groundPlane, refrUV).rgb;

    //reflection
    vec3 reflDir = reflect(I, normalMapTBN);
    vec3 reflColor = texture(Skybox, reflDir).rgb;

    // mix reflection, refraction and fresnel
    vec3 color = mix(refrColor, reflColor, fresnel(fresnel_amount, normalMapTBN, -I));

    // slight water tint
    color = mix(color, source_color.rgb, fresnel(fresnel_amount, normalMapTBN, -I));

    // water foam
    float slopeFoam = pow(1.0 - max(normalMapTBN.y, 0.0), 4.0);
    float peakFoam = smoothstep(0.0, 0.15, WaveHeight);
    float foam = max(slopeFoam, peakFoam);
    foam = clamp(foam, 0.0, 1.0);
    color = mix(color, vec3(0.9, 0.95, 1.0), foam * foam_amount);

    FragColor = vec4(color, 1.0);
}