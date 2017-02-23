#include "stdafx.h"
#include "Emulator.h";

int Emulator::opcode_00() { NOP(); return 4; };
int Emulator::opcode_03() { CPU_16BIT_INC(m_RegisterBC.reg); return 8; };
int Emulator::opcode_04() { CPU_8BIT_INC(m_RegisterBC.hi); return 4; };
int Emulator::opcode_13() { CPU_16BIT_INC(m_RegisterDE.reg); return 8; };
int Emulator::opcode_18() { CPU_JUMP_IMMEDIATE(false, 0, false); return 8; };
int Emulator::opcode_21() { CPU_16BIT_LOAD(m_RegisterHL.reg); return 12; };
int Emulator::opcode_23() { CPU_16BIT_INC(m_RegisterHL.reg); return 8; };
int Emulator::opcode_2A() {  return 8; };
int Emulator::opcode_31() { CPU_16BIT_LOAD(m_StackPointer.reg); return 12; };
int Emulator::opcode_33() { CPU_16BIT_INC(m_StackPointer.reg); return 8; };
int Emulator::opcode_3E() { CPU_8BIT_LOAD(m_RegisterAF.hi); return 8; };
int Emulator::opcode_7C() { CPU_REG_LOAD(m_RegisterAF.hi, m_RegisterHL.hi); return 4; }
int Emulator::opcode_7D() { CPU_REG_LOAD(m_RegisterAF.hi, m_RegisterHL.lo); return 4; };
int Emulator::opcode_AF() { CPU_8BIT_XOR(m_RegisterAF.hi, m_RegisterAF.hi); return 4; };
int Emulator::opcode_C1() { CPU_STACK_POP(m_RegisterBC.reg); return 12; };
int Emulator::opcode_C3() { CPU_JUMP(false, 0, false); return 12; };
int Emulator::opcode_C9() { CPU_RETURN(); return 8; };
int Emulator::opcode_CD() { CPU_CALL(false, 0, false); return 12; };
int Emulator::opcode_D1() { CPU_STACK_POP(m_RegisterDE.reg); return 12; };
int Emulator::opcode_E0() { CPU_LDH_INTO_DATA(); return 12; };
int Emulator::opcode_EA() { CPU_LD_TO_ADDRESS(m_RegisterAF.hi);  return 16; };
int Emulator::opcode_F1() { CPU_STACK_POP(m_RegisterAF.reg); return 12; };
int Emulator::opcode_F5() { CPU_STACK_PUSH(m_RegisterAF.reg); return 16; };
int Emulator::opcode_E1() { CPU_STACK_POP(m_RegisterHL.reg); return 12; };
int Emulator::opcode_E5() { CPU_STACK_PUSH(m_RegisterHL.reg); return 16; };