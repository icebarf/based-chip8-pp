# based-chip8-pp
Yet another chip8 interpreter I write for fun.

## libchip8++
This is a header-only library I wrote for internal use that implements the chip8 system and instructions.
Everything is provided inside the `Chip8_core` namespace and should be used through it.

The user can provide their own implementation for instructions as a custom `decode()` is  required for the user to implement when use.

The documentation for this library is provided at [libchip8pp.rdseed.xyz](https://libchip8pp.rdseed.xyz).
