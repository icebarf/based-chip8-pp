# based-chip8-pp
Yet another chip8 interpreter I write for fun.


## libchip8++
This is a core header-only library that implements the chip8 system and instructions.
Everything is provided inside the `Chip8_core` namespace and should be used through it.

The user can provide their own implementation for instructions as a custom `decode()` is 
required for the user to implement in use.

Everything will be documented here soon.

## `Chip8_core::system`
`system` is a class representing a chip8 system and is independent of any external
dependencies except the C++ Standard Template Library.

Each private member is accessible through a Getter/Setter pair 
(along with some `chip8.*Inc()` and `chip8.*Dec()` functions for (inc|dec)rementing by 1).

#### Constructor
```cpp
#include <libchip8++.hpp>
#include <utilitiy>
#include <random>

int main(int argc, char** argv)
{
    std::random_device device;
    Chip8_core::system chip8(std::move(device)) // initialise with a std::random_device rvalue reference
                                                // required for instruction.

    ...

    return 0;

}
```

Constructor needs a `std::random_device` rvalue reference as parameter to construct
`std::default_random_engine` and `std::uniform_int_distribution<int>`.
