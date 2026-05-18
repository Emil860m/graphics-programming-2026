#include "RefractionApp.h"
#include "perlin.h"
int main()
{
    save_image(0.7f, "noise_maps/perlin_octaves1.png");
    save_image(1.0f, "noise_maps/perlin_octaves2.png");
    save_image(1.0f, "noise_maps/perlin_octaves3.png");
    RefractionApp refractionApp;
    return refractionApp.Run();
}
