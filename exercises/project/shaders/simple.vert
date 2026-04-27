layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out float Height;
out vec3 WorldPos;


uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;
void main()
{
    Height = VertexPosition.y;
    vec3 vp = normalize(VertexPosition);
    WorldPos = (WorldMatrix * vec4(VertexPosition.x, VertexPosition.y - 100, VertexPosition.z, 1.0)).xyz;
    gl_Position = ViewProjMatrix * WorldMatrix * vec4(VertexPosition, 1.0);
}