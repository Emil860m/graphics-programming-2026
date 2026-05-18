in vec3 WorldPos;
in vec2 UV;
in mat3 TBN;

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

uniform vec3 CameraPosition;
uniform float Time;


float fresnel(float amount, vec3 normal, vec3 view) {
    return pow((1.0 - max(dot(normalize(normal), normalize(view)), 0.0)), amount);
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
    vec2 uv_movement = movement_direction * Time * movement_strength;
    vec3 n1 = getNormal(Normal1, uv + uv_movement);
    vec3 n2 = getNormal(Normal2, uv - uv_movement);

    // combine
    vec3 normalMap = normalize(n1 + n2);
    normalMap = normalize(TBN * normalMap);

    // fresnel
    vec3 I = normalize(WorldPos - CameraPosition);
    float basic_fresnel = pow(1.0 - max(dot(-I, normalMap), 0.0), 1.0);


    //refraction
    float eta = 1.0 / 1.33;
    vec3 refrDir = refract(I, normalMap, eta);
    vec2 refrUV = refrDir.xy * 0.5 + 0.5;
    vec3 refrColor = texture(groundPlane, refrUV).rgb;

    //reflection
    vec3 reflDir = reflect(I, normalMap);
    vec3 reflColor = texture(Skybox, reflDir).rgb;
    // mix reflection, refraction and fresnel
    vec3 color = mix(refrColor, reflColor,0.5);
    color = color + (0.2 * basic_fresnel);
    // slight water tint
    color = mix(color, source_color.rgb, 0.3);
    FragColor = vec4(reflColor, 0.9);
    //FragColor = vec4(normalMap * 0.5 + 0.5, 1.0);
}