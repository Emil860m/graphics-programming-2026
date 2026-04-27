
out vec4 FragColor;

in float Height;

void main()
{
    float h = (Height + 8)/32.0f;	// shift and scale the height into a grayscale value
    FragColor = vec4(Height, Height, Height, 1.0);
}