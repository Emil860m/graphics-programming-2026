layout (location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec3 VertexTangent;
layout(location = 3) in vec2 VertexUV;
out vec3 WorldPos;
out vec2 UV;
out mat3 TBN;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

void main()
{
    vec3 N = normalize(mat3(WorldMatrix) * VertexNormal);
    vec3 T = normalize(mat3(WorldMatrix) * VertexTangent);
    T = normalize(T - N * dot(N, T));
    vec3 B = cross(T, N);

    TBN = mat3(T, B, N);
    vec4 worldPos = WorldMatrix * vec4(VertexPosition, 1.0);
    WorldPos = worldPos.xyz;
    UV = VertexUV;
    gl_Position = ViewProjMatrix * worldPos;
}