# libchip8++

This is a header-only library I wrote for internal use that implements the chip8 system and instructions.
Everything is provided inside the `Chip8_core` namespace and should be used through it.

## Usage

In seprate Translation Unit do the following:

    #define LIBCHIP8_IMPLEMENTATION_SOURCE
    #include <libchip8++.h>

Then in any other file where you wish to use the library simply

    #include <libchip8++.h>

After doing this you should be able to compile your project as you normally do.
Header-only libraries are great for this purpose as it is only a single translation
unit that is part of your source tree.


## STL Dependencies

Please note that `libchip8pp` depends on a few C++ STL headers and some C headers.
All of those headers are listed below.

    #include <array>
    #include <bitset>
    #include <cstring>
    #include <filesystem>
    #include <fstream>
    #include <random>

## Browsing this documentation

Browse the [Chip8_core](https://libchip8pp.rdseed.xyz/namespaceChip8__core.html) namespace
to see the provided implementation.

> Note: some details have been intentionally left out in the document because the reader
> need not know those details that have been used internally.
