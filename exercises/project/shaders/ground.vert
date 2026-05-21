layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexUV;

out vec2 UV;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;
void main()
{
    vec3 vp = VertexPosition;
    vp.z += 6;
    vp.y *= 2;
    UV = VertexUV;
    gl_Position = ViewProjMatrix * WorldMatrix * vec4(vp, 1.0);
}