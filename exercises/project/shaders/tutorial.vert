layout (location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 VertexUV;
out vec3 WorldPos;
out vec2 UV;
out mat3 TBN;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

void main()
{
    vec3 T = normalize(mat3(WorldMatrix) * vec3(1, 0, 0));
    vec3 B = normalize(mat3(WorldMatrix) * vec3(0, 0, 1));
    vec3 N = normalize(mat3(WorldMatrix) * vec3(0, 1, 0));

    TBN = mat3(T, B, N);
    vec4 worldPos = WorldMatrix * vec4(VertexPosition, 1.0);
    WorldPos = worldPos.xyz;
    UV = VertexUV;
    gl_Position = ViewProjMatrix * WorldMatrix * vec4(VertexPosition, 1.0);
}