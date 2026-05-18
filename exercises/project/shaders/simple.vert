layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexUV;

out float Height;
out vec3 WorldPos;
out vec3 Normal;
out vec2 UV;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;
void main()
{
    Height = VertexPosition.y;
    WorldPos = (WorldMatrix * vec4(VertexPosition.x, 1.0, VertexPosition.z, 1.0)).xyz;
    Normal = mat3(WorldMatrix) * VertexNormal;
    UV = VertexUV;
    gl_Position = ViewProjMatrix * WorldMatrix * vec4(VertexPosition, 1.0);
}