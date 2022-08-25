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

namespace Chip8_core {

namespace fs = std::filesystem;

enum Constants {
    MEMSIZE = 4096,
    DISPW = 64,
    DISPH = 32,
    STACKSIZE = 48,
    REGCNT = 16,
    PROGRAM_LD_ADDR = 0x200,
    INIT_STACK_TOP = -1
};

struct system {
    std::array<std::uint8_t, Constants::MEMSIZE> memory;
    std::bitset<Constants::DISPW * Constants::DISPH> display;
    std::array<std::uint16_t, Constants::STACKSIZE> stack;
    std::array<std::uint8_t, Constants::REGCNT> registers;
    std::uint16_t index_reg;
    std::uint16_t program_counter;
    std::uint8_t delay_timer;
    std::uint8_t sound_timer;
    std::uint8_t stacktop;

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

    void load_rom(fs::path rom)
    {
        assert(fs::exists(rom));
        assert(fs::is_regular_file(rom));

        std::ifstream rs{ rom, std::ios::binary };
        assert(rs.is_open());

        rs.seekg(0, std ::ios::end);
        long size = rs.tellg();
        rs.seekg(0, std::ios::beg);
        rs.read(reinterpret_cast<char*>(&memory[Constants::PROGRAM_LD_ADDR]), size);
        assert(rs.gcount() == size);
    }
};

} // namespace Chip8_core

#endif