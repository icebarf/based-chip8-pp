#include <libchip8++.hpp>
#include <SDL.h>

int
main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stdout, "./chip8 [file]\n");
        return 1;
    }

    Chip8_core::system Chip8(std::random_device{});
    Chip8.LoadRom(argv[1]); // will terminate on failure
    fprintf(stdout, "%x\n", Chip8_core::Instructions::fetch_nib1(0xfa00));
    SDL_Init(SDL_INIT_VIDEO);
    return 0;
}