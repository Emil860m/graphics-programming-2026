#include "RefractionApp.h"
#include "perlin.h"
int main()
{
    //save_image(0.1f, "noise_maps/perlin_octaves1.png", 50);
    //save_image(1.0f, "noise_maps/perlin_octaves2.png", 50);
    //save_image(1.0f, "noise_maps/perlin_octaves3.png", 50);
    RefractionApp refractionApp;
    return refractionApp.Run();
}
