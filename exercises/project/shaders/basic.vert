layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexUV;

out vec3 WorldPos;
out vec3 Normal;
out vec2 UV;
out vec2 TexCoord;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

void main()
{
    vec4 worldPos = WorldMatrix * vec4(VertexPosition, 1.0);
    WorldPos = worldPos.xyz;

    Normal = mat3(WorldMatrix) * VertexNormal;

    // Generate UVs from position
    UV = VertexUV;

    gl_Position = ViewProjMatrix * worldPos;
    TexCoord = (gl_Position.xy / gl_Position.w) * 0.5f + 0.5f;
}