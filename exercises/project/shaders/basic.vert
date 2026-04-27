layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out vec3 WorldPos;
out vec3 Normal;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

void main()
{
    vec4 worldPos = WorldMatrix * vec4(VertexPosition, 1.0);
    WorldPos = worldPos.xyz;

    Normal = mat3(WorldMatrix) * VertexNormal;

    gl_Position = ViewProjMatrix * worldPos;
}