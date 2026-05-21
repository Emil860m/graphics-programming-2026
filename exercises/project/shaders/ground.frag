
out vec4 FragColor;


in vec2 UV;

uniform sampler2D tex;
void main()
{
    FragColor = vec4(texture(tex, UV).rgb, 1.0);
}