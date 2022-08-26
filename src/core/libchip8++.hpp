/*
     This file is part of based-chip8-pp.

    based-chip8-pp is free software: you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation, either version 3 of the License, or (at your option) any later
   version.

    based-chip8-pp is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with
   based-chip8-pp. If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef BASED_CHIP8_CORE
#define BASED_CHIP8_CORE

#include <array>
#include <assert.h>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <random>
#include <sys/types.h>

namespace Chip8_core {

namespace fs = std::filesystem;

enum Constants {
    MEMSIZE = 4096,
    DISPW = 64,
    DISPH = 32,
    STACKSIZE = 48,
    REGCNT = 16,
    PROGRAM_LD_ADDR = 0x200,
    INIT_STACK_TOP = -1,
    KEYCOUNT = 16
};

// clang-format off
enum KeyCode {
    One,  Two, Three, Four,
    Five, Six, Seven, Eight,
    Nine, Ten, A,     B,
    C,    D,   E,     F
};

enum Registers {
    R0, R1, R2, R3,
    R4, R5, R6, R7,
    R8, R9, RA, RB,
    RC, RD, RE, RF,
};
// clang-format on

enum ShiftMode {
    OCTO,  // shift VX by cnt(VY) store in VX
    MATT,  // shift VY by 1 store in VX
    COWGOD // shift VX by 1 store in VX
};

class system {
  private:
    std::array<uint8_t, Constants::MEMSIZE> memory;
    std::bitset<Constants::DISPW * Constants::DISPH> display;
    std::array<uint16_t, Constants::STACKSIZE> stack;
    std::array<uint8_t, Constants::REGCNT> registers;
    std::bitset<Constants::KEYCOUNT> keys;
    uint16_t index_reg;
    uint16_t program_counter;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t stacktop;

  public:
    system()
    {
        /* put font data in memory */
        memory = { 0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70,
                   0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0,
                   0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0,
                   0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40,
                   0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0,
                   0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
                   0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0,
                   0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80 };

        display = { 0 };
        stack = { 0 };
        registers = { 0 };
        index_reg = 0;
        program_counter = Constants::PROGRAM_LD_ADDR;
        delay_timer = 0;
        sound_timer = 0;
        stacktop = Constants::INIT_STACK_TOP;
    }

    void LoadRom(fs::path rom)
    {
        assert(fs::exists(rom));
        assert(fs::is_regular_file(rom));

        std::ifstream rs{ rom, std::ios::binary };
        assert(rs.is_open());

        long size = fs::file_size(rom);
        rs.read(reinterpret_cast<char*>(&memory[Constants::PROGRAM_LD_ADDR]), size);
        assert(rs.gcount() == size);
    }

    uint16_t Fetch()
    {
        uint16_t opcode =
          (memory[program_counter] << 8) | memory[program_counter + 1];
        program_counter += 2;
        return opcode;
    }

    void SetRegister(Registers r, uint8_t v)
    {
        registers[r] = v;
    }

    uint8_t GetRegister(Registers r)
    {
        return registers[r];
    }

    void SetIndexRegister(uint16_t v)
    {
        index_reg = v;
    }

    uint16_t GetIndexRegister()
    {
        return index_reg;
    }

    void SetKey(KeyCode k, uint8_t v)
    {
        keys[k] = v;
    }

    uint8_t GetKey(KeyCode k)
    {
        return keys[k];
    }

    void Push(uint16_t v)
    {
        stack[++stacktop] = v;
    }

    uint16_t Pop()
    {
        return stack[stacktop--];
    }

    void SetPC(uint16_t v)
    {
        program_counter = v;
    }

    uint16_t GetPC()
    {
        return program_counter;
    }

    void IncPC()
    {
        program_counter++;
    }

    void DecPC()
    {
        program_counter--;
    }

    void SetDT(uint8_t v)
    {
        delay_timer = v;
    }

    uint8_t GetDT()
    {
        return delay_timer;
    }

    void IncDT()
    {
        delay_timer++;
    }

    void DecDT()
    {
        delay_timer--;
    }

    void SetST(uint8_t v)
    {
        sound_timer = v;
    }

    uint8_t GetST()
    {
        return sound_timer;
    }

    void IncST()
    {
        sound_timer++;
    }

    void DecST()
    {
        sound_timer--;
    }

    void SetPixel(uint16_t idx, uint8_t v)
    {
        display[idx] = v;
    }

    uint8_t GetPixel(uint16_t idx)
    {
        return display[idx];
    }

    uint8_t& operator[](long i)
    {
        assert(i > 0);
        assert(i < (Constants::MEMSIZE + 1));

        return memory[i];
    }

    void reset_display()
    {
        display.reset();
    }

    void reset_keys()
    {
        keys.reset();
    }
};

/* excatly what the function names denote
 * let a 16-bit number be denoted as
 * XXXX YYYY ZZZZ AAAA
 * then X - Y makes up the higher byte of the opcode
 * and  Z - A makes up the lower byte of the opcode*/
uint8_t
fetch_nib1(uint16_t opcode);

uint8_t
fetch_nib2(uint16_t opcode);

uint8_t
fetch_nib3(uint16_t opcode);

uint8_t
fetch_nib4(uint16_t opcode);

#ifndef LIBCHIP8_IMPLEMENTATION_SOURCE
#define LIBCHIP8_IMPLEMENTATION_SOURCE

/* utility functions */

inline uint8_t
nibble2byte(uint8_t un, uint8_t ln) // NOLINT(misc-definitions-in-headers)
{
    return un << 4 | ln;
}

uint8_t
fetch_nib1(uint16_t opcode) // NOLINT(misc-definitions-in-headers)
{
    return (opcode >> 12);
}

uint8_t
fetch_nib2(uint16_t opcode) // NOLINT(misc-definitions-in-headers)
{
    return ((opcode << 4) >> 8);
}

uint8_t
fetch_nib3(uint16_t opcode) // NOLINT(misc-definitions-in-headers)
{
    return ((opcode << 8) >> 12);
}

uint8_t
fetch_nib4(uint16_t opcode) // NOLINT(misc-definitions-in-headers)
{
    return ((opcode << 12) >> 12);
}

/** instructions **/
void
sys_addr(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    (void)opcode;
    (void)Chip8;
}

void
cls(system Chip8) // NOLINT(misc-definitions-in-headers)
{
    Chip8.reset_display();
}

void
ret(system Chip8) // NOLINT(misc-definitions-in-headers)
{
    Chip8.SetPC(Chip8.Pop());
}

void
jmp(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    Chip8.SetPC((fetch_nib2(opcode) << 8) |
                nibble2byte(fetch_nib3(opcode), fetch_nib4(opcode)));
}

void
call(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    Chip8.Push(Chip8.GetPC());
    jmp(opcode, Chip8);
}

void
skip_eq(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    if (Chip8.GetRegister(static_cast<Registers>(fetch_nib2(opcode))) ==
        nibble2byte(fetch_nib3(opcode), fetch_nib4(opcode))) {
        Chip8.IncPC();
        Chip8.IncPC();
    }
}

void
skip_noteq(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    if (Chip8.GetRegister(static_cast<Registers>(fetch_nib2(opcode))) !=
        nibble2byte(fetch_nib3(opcode), fetch_nib4(opcode))) {
        Chip8.IncPC();
        Chip8.IncPC();
    }
}

void
skip_xyeq(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    if (Chip8.GetRegister(static_cast<Registers>(fetch_nib2(opcode))) ==
        Chip8.GetRegister(static_cast<Registers>(fetch_nib3(opcode)))) {
        Chip8.IncPC();
        Chip8.IncPC();
    }
}

void
load(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    Chip8.SetRegister(static_cast<Registers>(fetch_nib2(opcode)),
                      nibble2byte(fetch_nib3(opcode), fetch_nib4(opcode)));
}

void
add(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    Chip8.SetRegister(rx,
                      Chip8.GetRegister(rx) +
                        nibble2byte(fetch_nib3(opcode), fetch_nib4(opcode)));
}

void
load_reg(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(rx, Chip8.GetRegister(ry));
}

void
ror(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(rx, Chip8.GetRegister(rx) | Chip8.GetRegister(ry));
}

void
rand(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(rx, Chip8.GetRegister(rx) & Chip8.GetRegister(ry));
}

void
rxor(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(rx, Chip8.GetRegister(rx) ^ Chip8.GetRegister(ry));
}

void
raddc(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));

    Chip8.SetRegister(Registers::RF, 0);
    if (UINT8_MAX - Chip8.GetRegister(rx) < Chip8.GetRegister(ry))
        Chip8.SetRegister(Registers::RF, 1);

    Chip8.SetRegister(rx, Chip8.GetRegister(rx) + Chip8.GetRegister(ry));
}

void
rsubc(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(Registers::RF, 0);

    if (Chip8.GetRegister(rx) > Chip8.GetRegister(ry))
        Chip8.SetRegister(Registers::RF, 1);

    Chip8.SetRegister(rx, Chip8.GetRegister(rx) - Chip8.GetRegister(ry));
}

void
rshift_right(ShiftMode mode, // NOLINT(misc-definitions-in-headers)
             uint16_t opcode,
             system Chip8)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(Registers::RF, 0);

    switch (mode) {
        case ShiftMode::OCTO:
            if (Chip8.GetRegister(rx) & 0b1) Chip8.SetRegister(Registers::RF, 1);
            Chip8.SetRegister(rx, Chip8.GetRegister(rx) >> Chip8.GetRegister(ry));
            break;

        case ShiftMode::MATT:
            if (Chip8.GetRegister(ry) & 0b1) Chip8.SetRegister(Registers::RF, 1);
            Chip8.SetRegister(rx, Chip8.GetRegister(ry) >> 1);
            break;

        case ShiftMode::COWGOD:
            if (Chip8.GetRegister(rx) & 0b1) Chip8.SetRegister(Registers::RF, 1);
            Chip8.SetRegister(rx, Chip8.GetRegister(rx) >> 1);
            break;
    }
}

void
rsubc_reverse(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(Registers::RF, 0);

    if (Chip8.GetRegister(rx) < Chip8.GetRegister(ry))
        Chip8.SetRegister(Registers::RF, 1);

    Chip8.SetRegister(rx, Chip8.GetRegister(ry) - Chip8.GetRegister(rx));
}

void
rshift_left(ShiftMode mode, // NOLINT(misc-definitions-in-headers)
            uint16_t opcode,
            system Chip8)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(Registers::RF, 0);

    auto shift = [&](Registers to_shift, uint8_t shift_cnt) {
        if (Chip8.GetRegister(to_shift) & 0b1000'0000)
            Chip8.SetRegister(Registers::RF, 1);
        Chip8.SetRegister(rx, Chip8.GetRegister(to_shift) << shift_cnt);
    };

    switch (mode) {
        case ShiftMode::OCTO:
            shift(rx, Chip8.GetRegister(ry));
            break;

        case ShiftMode::MATT:
            shift(ry, 1);
            break;

        case ShiftMode::COWGOD:
            shift(rx, 1);
            break;
    }
}

void
skip_rnoteq(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    if (Chip8.GetRegister(static_cast<Registers>(fetch_nib2(opcode))) !=
        Chip8.GetRegister(static_cast<Registers>(fetch_nib3(opcode)))) {
        Chip8.IncPC();
        Chip8.IncPC();
    }
}

void
load_idxr_addr(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    uint16_t addr =
      fetch_nib2(opcode) << 8 | nibble2byte(fetch_nib3(opcode), fetch_nib4(opcode));
    Chip8.SetIndexRegister(addr);
}

void
jmpr(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    uint16_t addr =
      fetch_nib2(opcode) << 8 | nibble2byte(fetch_nib3(opcode), fetch_nib4(opcode));
    Chip8.SetPC(Chip8.GetRegister(Registers::R0) + addr);
}

void
genrandom(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto tccpp_rand = []() {
        std::random_device device;
        std::default_random_engine engine{ device() };
        std::uniform_int_distribution<int> distb{ 0, 255 };
        return distb(engine);
    };
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    Chip8.SetRegister(
      rx, tccpp_rand() & nibble2byte(fetch_nib3(opcode), fetch_nib4(opcode)));
}

void
draw(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    /* tobias vl's dxyn impl*/
    uint8_t val_x =
      Chip8.GetRegister(static_cast<Registers>(fetch_nib2(opcode))) & 63;
    uint8_t val_y =
      Chip8.GetRegister(static_cast<Registers>(fetch_nib3(opcode))) & 31;
    uint8_t N = fetch_nib4(opcode);
    Chip8.SetRegister(Registers::RF, 0);

    for (int rows = 0; rows < N; rows++) {
        uint8_t sprite = Chip8[Chip8.GetIndexRegister() + rows];

        val_y += rows;

        for (int col = 0; col < 8; col++) {
            val_x += col;

            if (sprite & (0b1000'0000 >> col)) {
                if (Chip8.GetPixel(val_x + val_y * Constants::DISPW))
                    Chip8.SetPixel(val_x + val_y * Constants::DISPW, 0);
                Chip8.SetPixel((rows + val_x) + (col + val_y) * Constants::DISPW, 1);
            }

            if (val_x >= Constants::DISPW) break;
        }
        if (val_y >= Constants::DISPH) break;
    }
}

#endif

} // namespace Chip8_core

#endif