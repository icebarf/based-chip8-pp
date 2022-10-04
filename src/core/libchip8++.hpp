/*
     This file is part of based-chip8-pp.

    based-chip8-pp is free software: you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation, either version 3 of the License, or (at your option) any
   later version.

    based-chip8-pp is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
   details.

    You should have received a copy of the GNU General Public License along with
   based-chip8-pp. If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef BASED_CHIP8_CORE
#define BASED_CHIP8_CORE

#include <algorithm>
#include <array>
#include <bitset>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>

/**
 * The main namsepace under which the whole implementation
 * for the chip8 core system is provided. It contains several
 * enumerations, a single system class, a few utility functions
 * for the implementation.
 */
namespace Chip8_core {

namespace fs = std::filesystem;

/**
 * Defines several constants used throughout
 * the implementation of instructions.
 */
enum Constants {
    MEMSIZE = 4096,          /**< The Chip8 internal memory size. */
    DISPW = 64,              /**< The Chip8 internal display width. */
    DISPH = 32,              /**< The Chip8 internal display height. */
    STACKSIZE = 48,          /**< The Chip8 internal stack height.  */
    REGCNT = 16,             /**< The Chip8 internal register count. */
    PROGRAM_LD_ADDR = 0x200, /**< The address at which Chip8 ROMs are loaded. */
    INIT_STACK_TOP = -1,     /**< The default value for stack top. */
    KEYCOUNT = 16,           /**< The number of keys present in Chip8. */
    ROM_MAX_SIZE =
      3215 /**< Maximum number of bytes allowed to be loaded in memory */
};

// clang-format off

/**
 * Contains named constants for Chip8 Keys.
 */
enum KeyCode {
    Zero,  One,  Two, Three,
    Four,  Five, Six, Seven,
    Eight, Nine, A,   B,
    C,     D,    E,   F
};

/**
 * Contains named constants for Chip8 Registers.
 */
enum Registers {
    R0, R1, R2, R3,
    R4, R5, R6, R7,
    R8, R9, RA, RB,
    RC, RD, RE, RF,
};
// clang-format on

/**
 * Contains named constants for various Chip8 instruction quriks.
 */
enum Quirks {
    MATT,   /**< Follow Matt Mikolay's chip8 reference.
               https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Instruction-Set
             */
    COWGOD, /**< Follow Eric Bryntse aka Cowgod's chip8 reference.
               http://devernay.free.fr/hacks/chip8/C8TECH10.HTM */
};

/**
 * Contains named constants for representing pressed and
 * not pressed keystate. Usually used internally maybe used in the frontend.
 */
enum Key {
    UP,  /**< The key is not pressed. */
    DOWN /**< The key is pressed. */
};

/**
 * Represents the entire Chip8 internal state.
 * It provides access to private data through pairs of Getters and Setters.
 * There are some methods that allow access to internal data via Reference.
 * These should not be used anywhere in the frontend as it can mess with the
 * state. These exist only to ease the implementation of instructions
 * internally.
 */
class system {
  private:
    std::array<uint8_t, Constants::MEMSIZE> memory;
    std::array<uint32_t, Constants::DISPW * Constants::DISPH> display;
    std::array<uint16_t, Constants::STACKSIZE> stack;
    std::array<uint8_t, Constants::REGCNT> registers;
    std::bitset<Constants::KEYCOUNT> keys;
    uint16_t index_reg;
    uint16_t program_counter;
    uint8_t delay_timer;
    uint8_t sound_timer;
    int8_t stacktop;
    std::default_random_engine engine;
    std::uniform_int_distribution<int> distb;

  public:
    uint32_t display_fg; /**< Foreground color */
    uint32_t display_bg; /**< Background color */
    /**
     * Constructs the Chip8 class.
     * This constructor loads the font into memory at address 0x0, sets the
     * program_counter to 0x200 which is the program load address, sets the
     * stack top, initialises random number generation engine with device
     * argument, sets the distribution between 0 and 255 (inclusive) and zero
     * initialises the rest.
     * @param device reference to a std::random_device for generating random
     * numbers.
     * @see Constants
     */
    system(std::random_device&& device,
           uint32_t foreground = 0xffffffff,
           uint32_t background = 0x0)
      : memory{ 0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70,
                0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0,
                0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0,
                0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40,
                0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0,
                0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
                0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0,
                0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80 }
      , display{ 0 }
      , stack{ 0 }
      , registers{ 0 }
      , keys{ 0 }
      , index_reg{ 0 }
      , program_counter{ Constants::PROGRAM_LD_ADDR }
      , delay_timer{ 0 }
      , sound_timer{ 0 }
      , stacktop{ Constants::INIT_STACK_TOP }
      , engine{ device() }
      , distb{ 0, 255 }
      , display_fg{ foreground }
      , display_bg{ background }
    {
    }

    /**
     * Loads the rom into memory at address 0x200 which is the program load
     * address.
     * @param rom path to the rom file, use like this
     * Chip8.LoadRom("games/chip8_rom.ch8")
     * @see Constants
     */
    void LoadRom(fs::path rom) noexcept
    {
        if (fs::exists(rom) == false) {
            fprintf(stderr, "file: %s does not exist.\n", rom.c_str());
            std::exit(1);
        }
        if (fs::is_regular_file(rom) == false) {
            fprintf(
              stderr,
              "file: '%s' is not a regular file. Will not attempt to read.\n",
              rom.c_str());
            std::exit(1);
        }

        std::ifstream rs{ rom, std::ios::binary };
        if (rs.is_open() == false) {
            fprintf(stderr,
                    "std::ifstream did not 'open()' file '%s'\n",
                    rom.c_str());
            std::exit(1);
        }

        long size = fs::file_size(rom);
        if (size >= Constants::ROM_MAX_SIZE) {
            fprintf(stderr,
                    "file: %s has a size larger than %d which is maximum "
                    "accepted file size\n",
                    rom.c_str(),
                    Constants::ROM_MAX_SIZE);
            std::exit(1);
        }
        rs.read(reinterpret_cast<char*>(&memory[Constants::PROGRAM_LD_ADDR]),
                size);
        if (rs.gcount() != size) {
            fprintf(
              stderr,
              "std::ifstream.read() did not read the specified file in its "
              "entirety.\nFile: %s\nFile size: %ld\nBytes Read: %ld\n",
              rom.c_str(),
              size,
              rs.gcount());
            std::exit(1);
        }
    }

    /**
     * Fetches an 16-bit opcode from memory. Also increments the program_counter
     * by 2.
     * @return 16-bit opcode containing a full chip8 instruction
     */
    uint16_t Fetch()
    {
        uint16_t opcode =
          (memory[program_counter] << 8) | memory[program_counter + 1];
        program_counter += 2;
        return opcode;
    }

    /**
     * Returns a reference to private data member memory.
     * @return reference to chip8 memory
     */
    std::array<uint8_t, Constants::MEMSIZE>& RefMemory()
    {
        return memory;
    }

    /**
     * Sets a register to some value.
     * @param r the register to modify, like SetRegister(0xA, 0x288) or
     * SetRegister(Registers::RA, 0x288)
     * @param v the value to set the register to.
     */
    void SetRegister(Registers r, uint8_t v)
    {
        registers[r] = v;
    }

    /**
     * Returns the value stored in a register.
     * @param r the register to get the value of, like GetRegitser(0xA) or
     * GetRegister(Registers::RA)
     */
    uint8_t GetRegister(Registers r)
    {
        return registers[r];
    }

    /**
     * Returns a reference to private data member registers.
     * @return reference to chip8 register array
     */
    std::array<uint8_t, Constants::REGCNT>& RefRegisterArray()
    {
        return registers;
    }

    /**
     * Set the value of the index register.
     * @param v the value to set
     */
    void SetIndexRegister(uint16_t v)
    {
        index_reg = v;
    }

    /**
     * Returns the value stored in index register.
     * @return 16-bit value. Probably an address
     */
    uint16_t GetIndexRegister()
    {
        return index_reg;
    }

    /**
     * Set a key to a UP or DOWN.
     * @param k KeyCode representing the key to set
     * @param v A value containing either Key::UP or Key::DOWN
     */
    void SetKey(KeyCode k, uint8_t v)
    {
        keys[k] = v;
    }

    /**
     * Returns the value to which a key is set.
     * @param k KeyCode representing the key
     * @return either Key::UP or Key::DOWN
     */
    uint8_t GetKey(KeyCode k)
    {
        return keys[k];
    }

    /**
     * Push an address to the stack. Also increment the stacktop
     * @param v the address to push
     */
    void Push(uint16_t v)
    {
        stack[++stacktop] = v;
    }

    /**
     * Pop off a value from the top of the stack and return it. Also decrement
     * the stacktop
     * @return A 16-bit value containig the address from top
     */
    uint16_t Pop()
    {
        return stack[stacktop--];
    }

    /**
     * Set the program_counter to some address.
     * @param v the address to set
     */
    void SetPC(uint16_t v)
    {
        program_counter = v;
    }

    /**
     * Returns the current value of program_counter.
     * @return A 16-bit address
     */
    uint16_t GetPC()
    {
        return program_counter;
    }

    /**
     * Increment the program_counter by 1.
     */
    void IncPC()
    {
        program_counter++;
    }

    /**
     * Decrement the program_counter by 1.
     */
    void DecPC()
    {
        program_counter--;
    }

    /**
     * Set the delay_timer to some value.
     * @param v the value to set
     */
    void SetDT(uint8_t v)
    {
        delay_timer = v;
    }

    /**
     * Get the current value of delay_timer.
     * @return value of the delay timer
     */
    uint8_t GetDT()
    {
        return delay_timer;
    }

    /**
     * Increment the delay timer by 1
     */
    void IncDT()
    {
        delay_timer++;
    }

    /**
     * Decrement the delay timer by 1
     */
    void DecDT()
    {
        delay_timer--;
    }

    /**
     * Set the sound_timer to some value.
     * @param v the value to set
     */
    void SetST(uint8_t v)
    {
        sound_timer = v;
    }

    /**
     * Get the current value of sound_timer.
     * @return value of the sound timer
     */
    uint8_t GetST()
    {
        return sound_timer;
    }

    /**
     * Increment the sound timer by 1
     */
    void IncST()
    {
        sound_timer++;
    }

    /**
     * Decrement the sound timer by 1
     */
    void DecST()
    {
        sound_timer--;
    }

    /**
     * Set a pixel in the display array to either 1 or 0.
     * @param idx the index at which the pixel resides, computed as (column +
     * (row * 64))
     * @param v the RGBA value to set pixel to
     */
    void SetPixel(uint16_t idx, uint32_t v)
    {
        display[idx] = v;
    }

    /**
     * Get the value of a pixel in the display array.
     * @param idx the index at which the pixel resides, computed as (column +
     * (row * 64))
     * @return the RGBA value of pixel at idx
     */
    uint32_t GetPixel(uint16_t idx)
    {
        return display[idx];
    }

    /**
     * Subscript operator overload allowing access to memory array of Chip8
     * class.
     */
    uint8_t& operator[](long i)
    {
        if (i > 0) {
            fprintf(stderr,
                    "Negative argument to subscript operator for class "
                    "Chip8_core::system");
            std::exit(1);
        }
        if (i < (Constants::MEMSIZE + 1)) {
            fprintf(stderr,
                    "Too large of an argument to subscript operator for class "
                    "Chip8_core::system. Argument should be in range [%d,%d)\n",
                    0,
                    Constants::MEMSIZE);
            std::exit(1);
        }

        return memory[i];
    }

    /**
     * Reset the entire display i.e set all pixels to 0 (UNSET)
     */
    void reset_display()
    {
        std::memset(&display[0], 0, Constants::DISPH * Constants::DISPW);
    }

    /**
     * Reset all the keys i.e set all keys to Key::UP (un-pressed)
     */
    void reset_keys()
    {
        keys.reset();
    }

    /**
     *  Returns a random integer between 0 and 255.
     * @return unsigned 8 bit random integer
     */
    uint8_t InternalRand()
    {
        return distb(engine);
    }
};

/**
 * Subnamespace inside the Chip8_core namespace.
 * Contains all declarations and definitions of Chip8 instructions.
 */
namespace Instructions {

/**
 * 00E0 - Clear the display.
 */
void
cls(system Chip8);

/**
 * 00EE - return.
 */
void
ret(system Chip8);

/**
 * 1NNN - jump to NNN.
 */
void
jmp(uint16_t opcode, system Chip8);

/**
 * 2NNN - call subroutine at NNN.
 */
void
call(uint16_t opcode, system Chip8);

/**
 * 3XNN - if RX != NN then do.
 */
void
skip_eq(uint16_t opcode, system Chip8);

/**
 * 4XNN - if RX == NN then do.
 */
void
skip_noteq(uint16_t opcode, system Chip8);

/**
 * 5XY0 - if RX != RY then do.
 */
void
skip_xyeq(uint16_t opcode, system Chip8);
void

/**
 * 6XNN - RX := NN.
 */
load(uint16_t opcode, system Chip8);

/**
 * 7XNN - RX += NN.
 */
void
add(uint16_t opcode, system Chip8);

/**
 * 8XY0 - RX := NN.
 */
void
load_reg(uint16_t opcode, system Chip8);

/**
 * 8XY1 - RX |= RY.
 */
void
regor(uint16_t opcode, system Chip8);

/**
 * 8XY2 - RX &= RY.
 */
void
regand(uint16_t opcode, system Chip8);

/**
 * 8XY3 - RX &= RY.
 */
void
regxor(uint16_t opcode, system Chip8);

/**
 * 8XY4 - RX ^= RY.
 */
void
regaddc(uint16_t opcode, system Chip8);

/**
 * 8XY5 - RX += RY.
 */
void
regsubc(uint16_t opcode, system Chip8);

/**
 * 8XY6 - RX >>= RY.
 *
 * Pass mode as Quirks::SHIFT_RY or Quirks::SHIFT_RX
 * for enabling the behaviour as described at enum Quriks.
 */
void
regshift_right(Quirks mode, uint16_t opcode, system Chip8);

/**
 * 8XY7 - RX = RY - RX.
 */
void
regsubc_reverse(uint16_t opcode, system Chip8);

/**
 * 8XYE - RX <<= RY.
 *
 * Pass mode as Quirks::SHIFT_RY or Quirks::SHIFT_RX
 * for enabling the behaviour as described at enum Quriks.
 */
void
regshift_left(Quirks mode, uint16_t opcode, system Chip8);

/**
 * 9XY0 - if RX == RY then do.
 */
void
skip_regnoteq(uint16_t opcode, system Chip8);

/**
 * ANNN - I := NNN.
 */
void
load_idxreg_addr(uint16_t opcode, system Chip8);

/**
 * BNNN - JMP (R0 + NNN).
 */
void
jmpreg(uint16_t opcode, system Chip8);

/**
 * CXNN - RX = Random_number & NN.
 * Note &:bitwise AND - similar to 8XY1,2,3 which are also bitwise operations.
 */
void
genrandom(uint16_t opcode, system Chip8);

/**
 * DXYN - Draw a sprite at Co-ordinates RX,RY of height N.
 * Note &:bitwise AND - similar to 8XY1,2,3 which are also bitwise operations.
 */
void
draw(uint16_t opcode, system Chip8);

/**
 * EX9E - if Keys[RX] set to Key::DOWN then do.
 */
void
skip_ifkeypress(uint16_t opcode, system Chip8);

/**
 * EXA1 - if Keys[RX] set to Key::UP then do.
 */
void
skip_ifkeynotpress(uint16_t opcode, system Chip8);

/**
 * FX07 - VX := Delay Timer.
 */
void
load_dt_to_reg(uint16_t opcode, system Chip8);

/**
 * FX0A - Wait for keypress, upon pressing load that key to RX.
 */
void
load_key(uint16_t opcode, system Chip8);

/**
 * FX15 - Delay Timer := RX.
 */
void
set_dt(uint16_t opcode, system Chip8);

/**
 * FX18 - Sound Timer := RX.
 */
void
set_st(uint16_t opcode, system Chip8);

/**
 * FX1E - Index += RX.
 */
void
regadd_idx(uint16_t opcode, system Chip8);

/**
 * FX29 - Set Index register to the location of a sprite in font memory.
 */
void
sprite(uint16_t opcode, system Chip8);

/**
 * FX33 - Decode RX into Binary Coded Decimal.
 */
void
decode_bcd(uint16_t opcode, system Chip8);

/**
 * FX55 - Save R0 to RX into memory[index] and onwards.
 * Pass mode = Quirks::MATT to follow Matt mikolay's documentation
 */
void
load_reg_into_memory(Quirks mode, uint16_t opcode, system Chip8) noexcept;

/**
 * FX65 - Save memory[Index] to memory[Index + X] into R0 and onwards.
 * Pass mode = Quirks::MATT to follow Matt mikolay's documentation
 */
void
load_memory_into_reg(Quirks mode, uint16_t opcode, system Chip8) noexcept;

/** @defgroup Opcode Utilities
 * The functions described here extract specific nibble from 16-bit opcode.
 * Let a 16-bit opcode be represented as the follows on a little endian machine.
 * XXXX YYYY AAAA BBBB .
 * then XXXX is the first nibble, YYYY is the second nibble and thus so on.
 * @{
 */

/**
 * returns the first nibble from opcode.
 * @param opcode the 16-bit opcode
 * @return the first nibble stored in a uint8_t
 */
uint8_t
fetch_nib1(uint16_t opcode);

/**
 * returns the second nibble from opcode.
 * @param opcode the 16-bit opcode
 * @return the second nibble stored in a uint8_t
 */
uint8_t
fetch_nib2(uint16_t opcode);

/**
 * returns the third nibble from opcode.
 * @param opcode the 16-bit opcode
 * @return the third nibble stored in a uint8_t
 */
uint8_t
fetch_nib3(uint16_t opcode);

/**
 * returns the fourth nibble from opcode.
 * @param opcode the 16-bit opcode
 * @return the fourth nibble stored in a uint8_t
 */
uint8_t
fetch_nib4(uint16_t opcode);

/**
 * converts both nibble parameters to a single byte.
 * @param un the upper nibble of an 8 bit byte
 * @param ln the lower nibble of an 8 bit byte
 * @return a single 8-bit byte comprising of un << 4 | ln
 */
uint8_t
nibble2byte(uint8_t un, uint8_t ln);

/** @} */ // end the Opcode Utilities group here
#ifdef LIBCHIP8_IMPLEMENTATION_SOURCE
#undef LIBCHIP8_IMPLEMENTATION_SOURCE

/* utility functions */
/* excatly what the function names denote
 * let a 16-bit number be denoted as
 * XXXX YYYY ZZZZ AAAA
 * then X - Y makes up the higher byte of the opcode
 * and  Z - A makes up the lower byte of the opcode
 */
uint8_t
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
        nibble2byte(fetch_nib3(opcode), fetch_nib4(opcode)))
        Chip8.SetPC(Chip8.GetPC() + 2);
}

void
skip_noteq(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    if (Chip8.GetRegister(static_cast<Registers>(fetch_nib2(opcode))) !=
        nibble2byte(fetch_nib3(opcode), fetch_nib4(opcode)))
        Chip8.SetPC(Chip8.GetPC() + 2);
}

void
skip_xyeq(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    if (Chip8.GetRegister(static_cast<Registers>(fetch_nib2(opcode))) ==
        Chip8.GetRegister(static_cast<Registers>(fetch_nib3(opcode))))
        Chip8.SetPC(Chip8.GetPC() + 2);
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
regor(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(rx, Chip8.GetRegister(rx) | Chip8.GetRegister(ry));
}

void
regand(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(rx, Chip8.GetRegister(rx) & Chip8.GetRegister(ry));
}

void
regxor(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(rx, Chip8.GetRegister(rx) ^ Chip8.GetRegister(ry));
}

void
regaddc(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));

    Chip8.SetRegister(Registers::RF, 0);
    if (UINT8_MAX - Chip8.GetRegister(rx) < Chip8.GetRegister(ry))
        Chip8.SetRegister(Registers::RF, 1);

    Chip8.SetRegister(rx, Chip8.GetRegister(rx) + Chip8.GetRegister(ry));
}

void
regsubc(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(Registers::RF, 0);

    if (Chip8.GetRegister(rx) > Chip8.GetRegister(ry))
        Chip8.SetRegister(Registers::RF, 1);

    Chip8.SetRegister(rx, Chip8.GetRegister(rx) - Chip8.GetRegister(ry));
}

void
regshift_right(Quirks mode, // NOLINT(misc-definitions-in-headers)
               uint16_t opcode,
               system Chip8)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(Registers::RF, 0);

    auto shift = [&](Registers to_shift, uint8_t shift_cnt) {
        if (Chip8.GetRegister(to_shift) & 0b1)
            Chip8.SetRegister(Registers::RF, 1);
        Chip8.SetRegister(rx, Chip8.GetRegister(to_shift) >> shift_cnt);
    };

    switch (mode) {
        /* Shift the RY register and store in RX */
        case Quirks::MATT:
            shift(ry, 1);
            break;

        /* Shift the RX register and store in RY */
        case Quirks::COWGOD:
            shift(rx, 1);
            break;
        default:
            break;
    }
}

void
regsubc_reverse(uint16_t opcode,
                system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    auto ry = static_cast<Registers>(fetch_nib3(opcode));
    Chip8.SetRegister(Registers::RF, 0);

    if (Chip8.GetRegister(rx) < Chip8.GetRegister(ry))
        Chip8.SetRegister(Registers::RF, 1);

    Chip8.SetRegister(rx, Chip8.GetRegister(ry) - Chip8.GetRegister(rx));
}

void
regshift_left(Quirks mode, // NOLINT(misc-definitions-in-headers)
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
        /* Shift the RY register and store in RX */
        case Quirks::MATT:
            shift(ry, 1);
            break;

        /* Shift the RX register and store in RY */
        case Quirks::COWGOD:
            shift(rx, 1);
            break;
        default:
            break;
    }
}

void
skip_regnoteq(uint16_t opcode,
              system Chip8) // NOLINT(misc-definitions-in-headers)
{
    if (Chip8.GetRegister(static_cast<Registers>(fetch_nib2(opcode))) !=
        Chip8.GetRegister(static_cast<Registers>(fetch_nib3(opcode))))
        Chip8.SetPC(Chip8.GetPC() + 2);
}

void
load_idxreg_addr(uint16_t opcode,
                 system Chip8) // NOLINT(misc-definitions-in-headers)
{
    uint16_t addr = fetch_nib2(opcode) << 8 |
                    nibble2byte(fetch_nib3(opcode), fetch_nib4(opcode));
    Chip8.SetIndexRegister(addr);
}

void
jmpreg(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    uint16_t addr = fetch_nib2(opcode) << 8 |
                    nibble2byte(fetch_nib3(opcode), fetch_nib4(opcode));
    Chip8.SetPC(Chip8.GetRegister(Registers::R0) + addr);
}

void
genrandom(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    Chip8.SetRegister(rx,
                      Chip8.InternalRand() &
                        nibble2byte(fetch_nib3(opcode), fetch_nib4(opcode)));
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
        if (val_y >= Constants::DISPH) break;

        for (int col = 0; col < 8; col++) {
            val_x += col;
            if (val_x >= Constants::DISPW) break;

            if (sprite & (0b1000'0000 >> col)) {
                if (Chip8.GetPixel(val_x + val_y * Constants::DISPW)) {
                    Chip8.SetPixel(val_x + val_y * Constants::DISPW,
                                   Chip8.display_bg);
                    Chip8.SetRegister(Registers::RF, 1);
                    continue;
                }

                Chip8.SetPixel(val_x + val_y * Constants::DISPW,
                               Chip8.display_fg);
            }
        }
    }
}

void
skip_ifkeypress(uint16_t opcode,
                system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto regval = Chip8.GetRegister(static_cast<Registers>(fetch_nib2(opcode)));
    if (Chip8.GetKey(static_cast<KeyCode>(regval)) == Key::DOWN)
        Chip8.SetPC(Chip8.GetPC() + 2);
}

void
skip_ifkeynotpress(uint16_t opcode, // NOLINT(misc-definitions-in-headers)
                   system Chip8)
{
    auto regval = Chip8.GetRegister(static_cast<Registers>(fetch_nib2(opcode)));
    if (Chip8.GetKey(static_cast<KeyCode>(regval)) == Key::UP)
        Chip8.SetPC(Chip8.GetPC() + 2);
}

void
load_dt_to_reg(uint16_t opcode,
               system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    Chip8.SetRegister(rx, Chip8.GetDT());
}

void
load_key(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    Chip8.SetPC(Chip8.GetPC() - 2);
    for (int i = 0x0; i <= 0xf; i++) {
        if (Chip8.GetKey(static_cast<KeyCode>(i)) == Key::DOWN) {
            Chip8.SetRegister(rx, i);
            Chip8.SetPC(Chip8.GetPC() + 2);
        }
    }
}

void
set_dt(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    Chip8.SetDT(Chip8.GetRegister(rx));
}

void
set_st(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    Chip8.SetST(Chip8.GetRegister(rx));
}

void
radd_idx(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    Chip8.SetIndexRegister(Chip8.GetIndexRegister() + Chip8.GetRegister(rx));
}

void
sprite(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    auto rx = static_cast<Registers>(fetch_nib2(opcode));
    Chip8.SetIndexRegister((Chip8.GetRegister(rx) % 16) *
                           5); // modulus 16 because we want hexadecimal value
}

void
decode_bcd(uint16_t opcode, system Chip8) // NOLINT(misc-definitions-in-headers)
{
    uint8_t num = Chip8.GetRegister(static_cast<Registers>(fetch_nib2(opcode)));
    Chip8[Chip8.GetIndexRegister() + 2] = num % 10; // ones place
    num = num / 10;
    Chip8[Chip8.GetIndexRegister() + 1] = num % 10; // tens place
    num = num / 10;
    Chip8[Chip8.GetIndexRegister()] = num; // hundreds place
}

void
load_reg_into_memory(Quirks mode, // NOLINT(misc-definitions-in-headers)
                     uint16_t opcode,
                     system Chip8) noexcept
{
    uint8_t last_reg{ fetch_nib2(opcode) };
    std::copy_n(Chip8.RefRegisterArray().begin(),
                last_reg,
                Chip8.RefMemory().begin() + Chip8.GetIndexRegister());

    /* According to Matt Mikolay's documentation
     * I is set to I + X + 1 after performing the operation
     */
    if (mode == Quirks::MATT)
        Chip8.SetIndexRegister(Chip8.GetIndexRegister() + last_reg + 1);
}

void
load_memory_into_reg(Quirks mode, // NOLINT(misc-definitions-in-headers)
                     uint16_t opcode,
                     system Chip8) noexcept
{
    uint8_t last_reg{ fetch_nib2(opcode) };
    std::copy_n(Chip8.RefMemory().begin() + Chip8.GetIndexRegister(),
                last_reg,
                Chip8.RefRegisterArray().begin());

    /* According to Matt Mikolay's documentation
     * I is set to I + X + 1 after performing the operation
     */
    if (mode == Quirks::MATT)
        Chip8.SetIndexRegister(Chip8.GetIndexRegister() + last_reg + 1);
}

#endif
} // namespace Instructions
} // namespace Chip8_core

#endif