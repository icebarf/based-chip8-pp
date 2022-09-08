# based-chip8-pp
Yet another chip8 interpreter I write for fun.

## libchip8++
This is a header-only library I wrote for internal use that implements the chip8 system and instructions.
Everything is provided inside the `Chip8_core` namespace and should be used through it.

The user can provide their own implementation for instructions as a custom `decode()` is  required for the user to implement when use.

The documentation for this library is provided at [libchip8pp.rdseed.xyz](https://libchip8pp.rdseed.xyz).

## Dependencies

- `CMake`: Build System
- `SDL2`: Backend for imgui
- `imgui`: The UI

> `imgui` will be fetched by CMake during configure phase

You must install `CMake` and `SDL2` on your system.

## Building

This is a short process that will compile the project. If you have all the dependencies
installed, everything should work smoothly as intended.

1. Clone the project or download the tarball then go into project directory
```
    git clone https://git.rdseed.xyz/based-chip8-pp.git
```

or

```
    git clone https://github.com/icebarf/based-chip8-pp.git
```

> Follow the step if you downloaded the tarball

```
    tar xvf based-chip-pp.tar.gz
```

2. Change directory
```
    cd based-chip8-pp
```

3. Configure the project with `CMake`
```
    cmake -S . -B build
```

4. Build it
```
    cmake --build build
```

After this, you should have an executable called `chip8` in the `build/` directory in project root.

## fuck this shit I'm out