in vec3 WorldPos;
in vec3 Normal;

out vec4 FragColor;

uniform samplerCube Skybox;
uniform vec3 CameraPosition;


/*
void main()
{    
    vec3 I = normalize(WorldPos - CameraPosition);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(Skybox, R).rgb, 1.0);
}
*/
///*
void main()
{             
    float ratio = 1.00 / 1.52;
    vec3 I = normalize(WorldPos - CameraPosition);
    vec3 R = refract(I, normalize(Normal), ratio);
    FragColor = vec4(texture(Skybox, R).rgb, 1.0);
}  
//*/