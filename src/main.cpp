#include <libchip8++.hpp>

int
main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stdout, "./chip8 [file]");
        return 1;
    }

    Chip8_core::system Chip8(std::random_device{});
    Chip8.LoadRom(argv[1]); // will terminate on failure
    return 0;
}