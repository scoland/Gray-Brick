#include "stdafx.h"
#include "Emulator.h";

int Emulator::opcode_00() { NOP(); return 4; }
int Emulator::opcode_01() { CPU_16BIT_LOAD(m_RegisterBC.reg); return 12; }
int Emulator::opcode_02() { writeMemory(m_RegisterBC.reg, m_RegisterAF.hi); return 8; }
int Emulator::opcode_03() { CPU_16BIT_INC(m_RegisterBC.reg); return 8; }
int Emulator::opcode_04() { CPU_8BIT_INC(m_RegisterBC.hi); return 4; }
int Emulator::opcode_05() { CPU_8BIT_DEC(m_RegisterBC.hi); return 4; }
int Emulator::opcode_06() { CPU_8BIT_LOAD(m_RegisterBC.hi); return 8; }
int Emulator::opcode_09() { CPU_ADD_HL(m_RegisterBC.reg); return 8; }
int Emulator::opcode_0A() { CPU_REG_LOAD_ROM(m_RegisterAF.hi, m_RegisterBC.reg); return 8; }
int Emulator::opcode_0C() { CPU_8BIT_INC(m_RegisterBC.lo); return 4; }
int Emulator::opcode_0D() { CPU_8BIT_DEC(m_RegisterBC.lo); return 4; }
int Emulator::opcode_0E() { CPU_8BIT_LOAD(m_RegisterBC.lo); return 8; }
int Emulator::opcode_11() { CPU_16BIT_LOAD(m_RegisterDE.reg); return 12; }
int Emulator::opcode_12() { writeMemory(m_RegisterDE.reg, m_RegisterAF.hi); return 8; }
int Emulator::opcode_13() { CPU_16BIT_INC(m_RegisterDE.reg); return 8; }
int Emulator::opcode_14() { CPU_8BIT_INC(m_RegisterDE.hi); return 4; }
int Emulator::opcode_15() { CPU_8BIT_DEC(m_RegisterDE.hi); return 4; }
int Emulator::opcode_16() { CPU_8BIT_LOAD(m_RegisterDE.hi); return 8; }
int Emulator::opcode_18() { CPU_JUMP_IMMEDIATE(false, 0, false); return 8; }
int Emulator::opcode_19() { CPU_ADD_HL(m_RegisterDE.reg); return 8; }
int Emulator::opcode_1A() { CPU_REG_LOAD_ROM(m_RegisterAF.hi, m_RegisterDE.reg); return 8; }
int Emulator::opcode_1C() { CPU_8BIT_INC(m_RegisterDE.lo); return 4; }
int Emulator::opcode_1D() { CPU_8BIT_DEC(m_RegisterDE.lo); return 4; }
int Emulator::opcode_1E() { CPU_8BIT_LOAD(m_RegisterDE.hi); return 8; }
int Emulator::opcode_1F() { CPU_RRC(m_RegisterAF.hi); return 4; }
int Emulator::opcode_20() { CPU_JUMP_IMMEDIATE(true, FLAG_Z, false); return 8; }
int Emulator::opcode_21() { CPU_16BIT_LOAD(m_RegisterHL.reg); return 12; }
int Emulator::opcode_23() { CPU_16BIT_INC(m_RegisterHL.reg); return 8; }
int Emulator::opcode_24() { CPU_8BIT_INC(m_RegisterHL.hi); return 4; }
int Emulator::opcode_25() { CPU_8BIT_DEC(m_RegisterHL.hi); return 4; }
int Emulator::opcode_26() { CPU_8BIT_LOAD(m_RegisterHL.hi); return 8; }
int Emulator::opcode_28() { CPU_JUMP_IMMEDIATE(true, FLAG_Z, true); return 8; }
int Emulator::opcode_29() { CPU_ADD_HL(m_RegisterHL.reg); return 8; }
int Emulator::opcode_2A() { CPU_LD_I(m_RegisterAF.hi, m_RegisterHL.reg); return 8; }
int Emulator::opcode_2C() { CPU_8BIT_INC(m_RegisterHL.lo); return 4; }
int Emulator::opcode_2D() { CPU_8BIT_DEC(m_RegisterHL.lo); return 4; }
int Emulator::opcode_2E() { CPU_8BIT_LOAD(m_RegisterHL.lo); return 8; }
int Emulator::opcode_30() { CPU_JUMP_IMMEDIATE(true, FLAG_C, false); return 8; }
int Emulator::opcode_31() { CPU_16BIT_LOAD(m_StackPointer.reg); return 12; }
int Emulator::opcode_33() { CPU_16BIT_INC(m_StackPointer.reg); return 8; }
int Emulator::opcode_35() { CPU_8BIT_MEMORY_DEC(m_RegisterHL.reg); return 12; }
int Emulator::opcode_39() { CPU_ADD_HL(m_StackPointer.reg); return 8; }
int Emulator::opcode_3C() { CPU_8BIT_INC(m_RegisterAF.hi); return 4; }
int Emulator::opcode_3D() { CPU_8BIT_DEC(m_RegisterAF.hi); return 4; }
int Emulator::opcode_3E() { CPU_8BIT_LOAD(m_RegisterAF.hi); return 8; }
int Emulator::opcode_4E() { CPU_REG_LOAD_ROM(m_RegisterBC.lo, m_RegisterHL.reg); return 8; }
int Emulator::opcode_4F() { CPU_REG_LOAD(m_RegisterBC.lo, m_RegisterAF.hi); return 4; }
int Emulator::opcode_46() { CPU_REG_LOAD_ROM(m_RegisterBC.hi, m_RegisterHL.reg); return 8; }
int Emulator::opcode_47() { CPU_REG_LOAD(m_RegisterBC.hi, m_RegisterAF.hi); return 4; }
int Emulator::opcode_56() { CPU_REG_LOAD_ROM(m_RegisterDE.hi, m_RegisterHL.reg); return 8; }
int Emulator::opcode_57() { CPU_REG_LOAD(m_RegisterDE.hi, m_RegisterAF.hi); return 4; }
int Emulator::opcode_5F() { CPU_REG_LOAD(m_RegisterDE.lo, m_RegisterAF.hi); return 4; }
int Emulator::opcode_67() { CPU_REG_LOAD(m_RegisterHL.hi, m_RegisterAF.hi); return 4; }
int Emulator::opcode_6E() { CPU_REG_LOAD_ROM(m_RegisterHL.lo, m_RegisterHL.reg); return 8; }
int Emulator::opcode_6F() { CPU_REG_LOAD(m_RegisterHL.lo, m_RegisterAF.hi); return 4; }
int Emulator::opcode_70() { writeMemory(m_RegisterHL.reg, m_RegisterBC.hi); return 8; }
int Emulator::opcode_71() { writeMemory(m_RegisterHL.reg, m_RegisterBC.lo); return 8; }
int Emulator::opcode_72() { writeMemory(m_RegisterHL.reg, m_RegisterDE.hi); return 8; }
int Emulator::opcode_73() { writeMemory(m_RegisterHL.reg, m_RegisterDE.lo); return 8; }
int Emulator::opcode_74() { writeMemory(m_RegisterHL.reg, m_RegisterHL.hi); return 8; }
int Emulator::opcode_75() { writeMemory(m_RegisterHL.reg, m_RegisterHL.lo); return 8; }
int Emulator::opcode_77() { writeMemory(m_RegisterHL.reg, m_RegisterAF.hi); return 8; }
int Emulator::opcode_78() { CPU_REG_LOAD(m_RegisterAF.hi, m_RegisterBC.hi); return 4; }
int Emulator::opcode_79() { CPU_REG_LOAD(m_RegisterAF.hi, m_RegisterBC.lo); return 4; }
int Emulator::opcode_7A() { CPU_REG_LOAD(m_RegisterAF.hi, m_RegisterDE.hi); return 4; }
int Emulator::opcode_7B() { CPU_REG_LOAD(m_RegisterAF.hi, m_RegisterDE.lo); return 4; }
int Emulator::opcode_7C() { CPU_REG_LOAD(m_RegisterAF.hi, m_RegisterHL.hi); return 4; }
int Emulator::opcode_7D() { CPU_REG_LOAD(m_RegisterAF.hi, m_RegisterHL.lo); return 4; }
int Emulator::opcode_7E() { CPU_REG_LOAD_ROM(m_RegisterAF.hi, m_RegisterHL.reg); return 8; }
int Emulator::opcode_7F() { CPU_REG_LOAD(m_RegisterAF.hi, m_RegisterAF.hi); return 4; }
int Emulator::opcode_80() { CPU_8BIT_ADD(m_RegisterAF.hi, m_RegisterBC.hi, false, false); return 4; }
int Emulator::opcode_81() { CPU_8BIT_ADD(m_RegisterAF.hi, m_RegisterBC.lo, false, false); return 4; }
int Emulator::opcode_82() { CPU_8BIT_ADD(m_RegisterAF.hi, m_RegisterDE.lo, false, false); return 4; }
int Emulator::opcode_83() { CPU_8BIT_ADD(m_RegisterAF.hi, m_RegisterDE.lo, false, false); return 4; }
int Emulator::opcode_84() { CPU_8BIT_ADD(m_RegisterAF.hi, m_RegisterHL.hi, false, false); return 4; }
int Emulator::opcode_85() { CPU_8BIT_ADD(m_RegisterAF.hi, m_RegisterHL.lo, false, false); return 4; }
int Emulator::opcode_86() { CPU_8BIT_ADD(m_RegisterAF.hi, readMemory(m_RegisterHL.reg), false, false); return 8; }
int Emulator::opcode_87() { CPU_8BIT_ADD(m_RegisterAF.hi, m_RegisterAF.hi, false, false); return 4; }
int Emulator::opcode_90() { CPU_8BIT_SUB(m_RegisterAF.hi, m_RegisterBC.hi, false, false); return 8; }
int Emulator::opcode_A0() { CPU_AND(m_RegisterBC.hi, false); return 4; }
int Emulator::opcode_A1() { CPU_AND(m_RegisterBC.lo, false); return 4; }
int Emulator::opcode_A2() { CPU_AND(m_RegisterDE.hi, false); return 4; }
int Emulator::opcode_A3() { CPU_AND(m_RegisterDE.lo, false); return 4; }
int Emulator::opcode_A4() { CPU_AND(m_RegisterHL.hi, false); return 4; }
int Emulator::opcode_A5() { CPU_AND(m_RegisterHL.lo, false); return 4; }
int Emulator::opcode_A7() { CPU_AND(m_RegisterAF.hi, false); return 4; }
int Emulator::opcode_A8() { CPU_8BIT_XOR(m_RegisterAF.hi, m_RegisterBC.hi, false); return 4; }
int Emulator::opcode_A9() { CPU_8BIT_XOR(m_RegisterAF.hi, m_RegisterBC.hi, false); return 4; }
int Emulator::opcode_AA() { CPU_8BIT_XOR(m_RegisterAF.hi, m_RegisterDE.hi, false); return 4; }
int Emulator::opcode_AB() { CPU_8BIT_XOR(m_RegisterAF.hi, m_RegisterDE.lo, false); return 4; }
int Emulator::opcode_AC() { CPU_8BIT_XOR(m_RegisterAF.hi, m_RegisterHL.hi, false); return 4; }
int Emulator::opcode_AD() { CPU_8BIT_XOR(m_RegisterAF.hi, m_RegisterHL.lo, false); return 4; }
int Emulator::opcode_AE() { CPU_8BIT_XOR(m_RegisterAF.hi, readMemory(m_RegisterHL.reg), false); return 8; }
int Emulator::opcode_AF() { CPU_8BIT_XOR(m_RegisterAF.hi, m_RegisterAF.hi, false); return 4; }
int Emulator::opcode_B0() { CPU_8BIT_OR(m_RegisterAF.hi, m_RegisterBC.hi, false); return 4; }
int Emulator::opcode_B1() { CPU_8BIT_OR(m_RegisterAF.hi, m_RegisterBC.lo, false); return 4; }
int Emulator::opcode_B2() { CPU_8BIT_OR(m_RegisterAF.hi, m_RegisterDE.hi, false); return 4; }
int Emulator::opcode_B3() { CPU_8BIT_OR(m_RegisterAF.hi, m_RegisterDE.lo, false); return 4; }
int Emulator::opcode_B4() { CPU_8BIT_OR(m_RegisterAF.hi, m_RegisterHL.hi, false); return 4; }
int Emulator::opcode_B5() { CPU_8BIT_OR(m_RegisterAF.hi, m_RegisterHL.lo, false); return 4; }
int Emulator::opcode_B6() { CPU_8BIT_OR(m_RegisterAF.hi, m_RegisterHL.reg, false); return 8; }
int Emulator::opcode_B7() { CPU_8BIT_OR(m_RegisterAF.hi, m_RegisterAF.hi, false); return 4; }
int Emulator::opcode_B8() { CPU_CP(m_RegisterAF.hi, m_RegisterBC.hi, false); return 4; }
int Emulator::opcode_B9() { CPU_CP(m_RegisterAF.hi, m_RegisterBC.lo, false); return 4; }
int Emulator::opcode_BA() { CPU_CP(m_RegisterAF.hi, m_RegisterDE.hi, false); return 4; }
int Emulator::opcode_BB() { CPU_CP(m_RegisterAF.hi, m_RegisterDE.lo, false); return 4; }
int Emulator::opcode_BC() { CPU_CP(m_RegisterAF.hi, m_RegisterHL.hi, false); return 4; }
int Emulator::opcode_BD() { CPU_CP(m_RegisterAF.hi, m_RegisterHL.lo, false); return 4; }
int Emulator::opcode_BF() { CPU_CP(m_RegisterAF.hi, m_RegisterAF.hi, false); return 4; }
int Emulator::opcode_C0() { CPU_RETURN(true, FLAG_Z, false); return 8; }
int Emulator::opcode_C1() { CPU_STACK_POP(m_RegisterBC.reg); return 12; }
int Emulator::opcode_C3() { CPU_JUMP(false, 0, false); return 12; }
int Emulator::opcode_C4() { CPU_CALL(true, FLAG_Z, false); return 12; }
int Emulator::opcode_C5() { CPU_STACK_PUSH(m_RegisterBC.reg); return 16; }
int Emulator::opcode_C6() { CPU_8BIT_ADD(m_RegisterAF.hi, 0, true, false); return 8; }
int Emulator::opcode_C8() { CPU_RETURN(true, FLAG_Z, true); return 8; }
int Emulator::opcode_C9() { CPU_RETURN(false, 0, false); return 8; }
int Emulator::opcode_CC() { CPU_CALL(true, FLAG_C, true); return 12; }
int Emulator::opcode_CD() { CPU_CALL(false, 0, false); return 12; }
int Emulator::opcode_CE() { CPU_ADC(get_byte_from_pc()); return 8; }
int Emulator::opcode_D0() { CPU_RETURN(true, FLAG_C, false); return 8; }
int Emulator::opcode_D1() { CPU_STACK_POP(m_RegisterDE.reg); return 12; }
int Emulator::opcode_D4() { CPU_CALL(true, FLAG_C, false); return 12; }
int Emulator::opcode_D5() { CPU_STACK_PUSH(m_RegisterDE.reg); return 16; }
int Emulator::opcode_D6() { CPU_8BIT_SUB(m_RegisterAF.hi, 0, true, false); return 8; }
int Emulator::opcode_D8() { CPU_RETURN(true, FLAG_C, true); return 8; }
int Emulator::opcode_DC() { CPU_CALL(true, FLAG_C, true); return 12; }
int Emulator::opcode_E0() { CPU_LDH_INTO_DATA(); return 12; }
int Emulator::opcode_E1() { CPU_STACK_POP(m_RegisterHL.reg); return 12; }
int Emulator::opcode_E5() { CPU_STACK_PUSH(m_RegisterHL.reg); return 16; }
int Emulator::opcode_E6() { CPU_AND(0, true); return 8; }
int Emulator::opcode_EA() { CPU_LD_TO_ADDRESS(m_RegisterAF.hi);  return 16; }
int Emulator::opcode_EE() { CPU_8BIT_XOR(m_RegisterAF.hi, 0, true); return 8; }
int Emulator::opcode_F1() { CPU_STACK_POP(m_RegisterAF.reg); return 12; }
int Emulator::opcode_F3() { m_InterruptMaster = false; return 4; }
int Emulator::opcode_F5() { CPU_STACK_PUSH(m_RegisterAF.reg); return 16; }
int Emulator::opcode_F6() { CPU_8BIT_OR(m_RegisterAF.hi, 0, true); return 8; }
int Emulator::opcode_FB() { m_InterruptMaster = true; return 4; }
int Emulator::opcode_FE() { CPU_CP(m_RegisterAF.hi, 0, true); return 8; }
int Emulator::opcode_CB_38() { CPU_SRL(m_RegisterBC.hi); return 8; }
int Emulator::opcode_CB_18() { CPU_RRC(m_RegisterBC.lo); return 8; }
int Emulator::opcode_CB_19() { CPU_RRC(m_RegisterBC.lo); return 8; }
int Emulator::opcode_CB_1A() { CPU_RRC(m_RegisterDE.hi); return 8; }
int Emulator::opcode_CB_1B() { CPU_RRC(m_RegisterDE.lo); return 8; }
int Emulator::opcode_CB_1C() { CPU_RRC(m_RegisterHL.hi); return 8; }
int Emulator::opcode_CB_1D() { CPU_RRC(m_RegisterHL.lo); return 8; }
int Emulator::opcode_CB_1E() { CPU_RRC_MEMORY(m_RegisterHL.reg); return 16; }
int Emulator::opcode_CB_1F() { CPU_RRC(m_RegisterAF.hi); return 8; }