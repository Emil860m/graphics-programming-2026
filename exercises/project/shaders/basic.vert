layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out vec3 WorldPos;
out vec3 Normal;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;
void main()
{
	// vertex position in world space (for lighting computation)
	WorldPos = (WorldMatrix * vec4(VertexPosition, 1.0)).xyz;

	// normal in world space (for lighting computation)
	Normal = (WorldMatrix * vec4(VertexNormal, 0.0)).xyz;


	// final vertex position (for opengl rendering, not for lighting)
	gl_Position = ViewProjMatrix * vec4(WorldPos, 1.0);
}
/*
void main()
{
    vec4 worldPos = WorldMatrix * vec4(VertexPosition, 1.0);
    WorldPos = worldPos.xyz;

    Normal = mat3(WorldMatrix) * VertexNormal;

    gl_Position = ViewProjMatrix * worldPos;
}
*/