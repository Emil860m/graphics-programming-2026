#include "RefractionApp.h"
#include "perlin.h"
int main()
{
    //save_image(0.5f, "noise_maps/perlin_octaves1.png", 100);
    //save_image(1.0f, "noise_maps/perlin_octaves2.png", 100);
    //save_image(1.0f, "noise_maps/perlin_octaves3.png", 100);
    RefractionApp refractionApp;
    return refractionApp.Run();
}
