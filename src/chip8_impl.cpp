#include "libchip8++.hpp"
#include <assert.h>

namespace c8 = Chip8_core;
namespace in = c8::Instructions;

void
cycle(c8::system& chip8, c8::Quirks mode)
{
    auto fetch_decode_execute = [&]() {
        uint16_t opcode = chip8.Fetch();
        switch (in::fetch_nib1(opcode)) {
            case 0x0:
                switch (in::nibble2byte(in::fetch_nib3(opcode),
                                        in::fetch_nib4(opcode))) {
                    case 0xE0:
                        in::cls(chip8);
                        break;

                    case 0xEE:
                        in::ret(chip8);
                        break;

                    default:
                        break;
                }

            case 0x1:
                in::jmp(opcode, chip8);
                break;

            case 0x2:
                in::call(opcode, chip8);
                break;

            case 0x3:
                in::skip_eq(opcode, chip8);
                break;

            case 0x4:
                in::skip_noteq(opcode, chip8);
                break;

            case 0x5:
                in::skip_xyeq(opcode, chip8);
                break;

            case 0x6:
                in::load(opcode, chip8);
                break;

            case 0x7:
                in::add(opcode, chip8);
                break;

            case 0x8:
                switch (in::fetch_nib4(opcode)) {
                    case 0x0:
                        in::load_reg(opcode, chip8);
                        break;

                    case 0x1:
                        in::regor(opcode, chip8);
                        break;

                    case 0x2:
                        in::regand(opcode, chip8);
                        break;

                    case 0x3:
                        in::regxor(opcode, chip8);
                        break;

                    case 0x4:
                        in::regaddc(opcode, chip8);
                        break;

                    case 0x5:
                        in::regsubc(opcode, chip8);
                        break;

                    case 0x6:
                        in::regshift_right(mode, opcode, chip8);
                        break;

                    case 0x7:
                        in::regsubc_reverse(opcode, chip8);
                        break;

                    case 0xE:
                        in::regshift_left(mode, opcode, chip8);
                        break;

                    default:
                        break;
                }

            case 0x9:
                /* assert the 9XY0 instruction format.
                 * Shouldn't happen but better be safe than sorry*/
                assert(in::fetch_nib4(opcode) == 0);

                in::load_idxreg_addr(opcode, chip8);
                break;

            case 0xA:
                in::load_idxreg_addr(opcode, chip8);
                break;

            case 0xB:
                in::jmpreg(opcode, chip8);
                break;

            case 0xC:
                in::genrandom(opcode, chip8);
                break;

            case 0xD:
                in::draw(opcode, chip8);
                break;

            case 0xE:
                switch (in::nibble2byte(in::fetch_nib3(opcode),
                                        in::fetch_nib4(opcode))) {
                    case 0x9E:
                        in::skip_ifkeypress(opcode, chip8);
                        break;

                    case 0xA1:
                        in::skip_ifkeynotpress(opcode, chip8);
                        break;
                }
                break;

            case 0xF:
                switch (in::nibble2byte(in::fetch_nib3(opcode),
                                        in::fetch_nib4(opcode))) {
                    case 0x07:
                        in::load_dt_to_reg(opcode, chip8);
                        break;

                    case 0x0A:
                        in::load_key(opcode, chip8);
                        break;

                    case 0x15:
                        in::set_dt(opcode, chip8);
                        break;

                    case 0x18:
                        in::set_st(opcode, chip8);
                        break;

                    case 0x1E:
                        in::regadd_idx(opcode, chip8);
                        break;

                    case 0x29:
                        in::sprite(opcode, chip8);
                        break;

                    case 0x33:
                        in::decode_bcd(opcode, chip8);
                        break;

                    case 0x55:
                        in::load_reg_into_memory(mode, opcode, chip8);
                        break;

                    case 0x65:
                        in::load_memory_into_reg(mode, opcode, chip8);
                        break;
                }
        }
    };
}