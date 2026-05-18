
out vec4 FragColor;
out vec4 FragAlbedo;
out vec2 FragNormal;
out vec4 FragOthers;

in float Height;
in vec3 Normal;
in vec2 UV;

uniform vec3 Color;
uniform sampler2D tex;
void main()
{
    //FragAlbedo =  vec4(Normal, 1.0);
    //FragNormal = normalize(Normal).xy;
    //FragOthers = vec4(0.0);
    vec3 color = texture(tex, UV).rgb;
    //color = mix(color, vec3(0.0, 0.25, 0.4), 0.1);
    FragColor = vec4(color, 1.0);
    //float h = (Height + 8)/32.0f;	// shift and scale the height into a grayscale value
    //FragColor = vec4(Height, Height, Height, 1.0);
}