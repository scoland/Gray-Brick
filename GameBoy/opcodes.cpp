#include "stdafx.h"
#include "Emulator.h";

#include <iostream>;

void Emulator::NOP()
{
	/* Do nothing! */
}

BYTE Emulator::get_byte_from_pc()
{
	BYTE n = readMemory(m_ProgramCounter);
	m_ProgramCounter++;

	return n;
}

WORD Emulator::get_word_from_pc()
{
	WORD nn = readWord();
	m_ProgramCounter += 2;

	return nn;
}

void Emulator::CPU_CALL(bool useCondition, int flag, bool condition)
{
	// grab immediate WORD value from after pc
	WORD nn = readWord();
	m_ProgramCounter += 2;

	if (!useCondition)
	{
		pushWordOntoStack(m_ProgramCounter);
		m_ProgramCounter = nn;
	}
}

void Emulator::CPU_JUMP(bool useCondition, int flag, bool condition)
{
	WORD nn = readWord();
	m_ProgramCounter += 2;

	if (!useCondition)
	{
		m_ProgramCounter = nn;
		return;
	}

	m_ProgramCounter++;
}

void Emulator::CPU_JUMP_IMMEDIATE(bool useCondition, int flag, bool condition)
{
	// Add n to current address and jump to it.

	SIGNED_BYTE n = (SIGNED_BYTE)readMemory(m_ProgramCounter);

	if (!useCondition)
	{
		m_ProgramCounter += n;
	}
	else if (ISBITSET(m_RegisterAF.lo, flag) == condition)
	{
		m_ProgramCounter += n;
	}

	m_ProgramCounter++;
}

void Emulator::CPU_8BIT_LOAD(BYTE& reg)
{
	BYTE byte = readMemory(m_ProgramCounter);
	m_ProgramCounter++;
	reg = byte;
}

void Emulator::CPU_16BIT_LOAD(WORD& reg) {
	WORD word = readWord();
	m_ProgramCounter += 2;
	reg = word;
}

void Emulator::CPU_REG_LOAD(BYTE& reg, BYTE value)
{
	reg = value;
}

void Emulator::CPU_REG_LOAD_ROM(BYTE& reg, WORD address)
{
	reg = readMemory(address);
}

void Emulator::CPU_8BIT_INC(BYTE& reg)
{
	BYTE before = reg;
	reg++;

	if (reg == 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_Z);
	else
		m_RegisterAF.lo = CLEARBIT(m_RegisterAF.lo, FLAG_Z);

	m_RegisterAF.lo = CLEARBIT(m_RegisterAF.lo, FLAG_N);

	if ((before & 0x0F) == 0xF)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_H);
	else
		m_RegisterAF.lo = CLEARBIT(m_RegisterAF.lo, FLAG_H);

}

void Emulator::CPU_8BIT_DEC(BYTE& reg)
{
	BYTE before = reg;
	reg--;

	if (reg == 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_Z);
	else
		m_RegisterAF.lo = CLEARBIT(m_RegisterAF.lo, FLAG_Z);

	m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_N);

	if ((before & 0x0F) == 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_H);
	else
		m_RegisterAF.lo = CLEARBIT(m_RegisterAF.lo, FLAG_H);
}

void Emulator::CPU_16BIT_INC(WORD& reg)
{
	reg++;
}

void Emulator::CPU_16BIT_DEC(WORD& reg)
{
	reg--;
}

void Emulator::CPU_8BIT_XOR(BYTE& reg, BYTE toXOR)
{
	reg ^= toXOR;

	// We reset all flags except possibly Z
	m_RegisterAF.lo = 0;

	if (reg == 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_Z);
}

void Emulator::CPU_LD_TO_ADDRESS(BYTE& reg)
{
	// get immediate 16 bit address value from PC
	WORD address = readWord();
	m_ProgramCounter += 2;

	// Write value in reg to that address
	writeMemory(address, reg);
}

void Emulator::CPU_LDH_INTO_DATA()
{
	BYTE val = readMemory(m_ProgramCounter);
	m_ProgramCounter++;

	BYTE address = 0xFF00 + val;
	writeMemory(address, m_RegisterAF.hi);
}

void Emulator::CPU_RETURN(bool useCondition, int flag, bool condition)
{

	if (!useCondition)
	{
		m_ProgramCounter = popWordOffStack();
		return;
	}

	// if flag value is equal to our condition, jump
	if (condition == ISBITSET(m_RegisterAF.hi, flag))
	{
		m_ProgramCounter = popWordOffStack();
		return;
	}
	
}

void Emulator::CPU_STACK_PUSH(WORD val)
{
	pushWordOntoStack(val);
}

void Emulator::CPU_STACK_POP(WORD& reg)
{
	reg = popWordOffStack();
}

void Emulator::CPU_LD_I(BYTE& reg, WORD& address)
{
	reg = readMemory(address);
	CPU_16BIT_INC(address);
}

void Emulator::CPU_8BIT_SUB(BYTE& reg, BYTE subtracting, bool useImmediate, bool subCarry)
{
	BYTE initial = reg;
	BYTE toSubtract = 0;

	// We can either use an immediate value or another register
	if (useImmediate)
	{
		BYTE n = readMemory(m_ProgramCounter);
		m_ProgramCounter++;
		toSubtract = n;
	}
	else
	{
		toSubtract = subtracting;
	}
	
	reg -= toSubtract;

	// reset flags
	m_RegisterAF.lo = 0;

	if (reg == 0) 
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_Z);

	m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_N);

	// set if no borrow
	if (initial < toSubtract)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_C);


	SIGNED_WORD htest = (initial & 0xF);
	htest -= (toSubtract & 0xF);

	if (htest < 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_H);
}

void Emulator::CPU_RRC(BYTE& reg)
{
	bool isLSBSet = ISBITSET(reg, 0);

	m_RegisterAF.lo = 0;

	reg >>= 1;

	if (isLSBSet)
	{
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_C);
		reg = SETBIT(reg, 7);
	}
	if (reg == 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_Z);
}

// Add n + Carry flag to A.
void Emulator::CPU_ADC(BYTE toAdd)
{
	BYTE& reg = m_RegisterAF.hi;

	BYTE initial = reg;
	int carry = ISBITSET(m_RegisterAF.lo, FLAG_C) ? 1 : 0;

	reg = reg + carry + toAdd;

	if (reg == 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_Z);
	else
		m_RegisterAF.lo = CLEARBIT(m_RegisterAF.lo, FLAG_Z);

	m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_N);

	if ((reg & 0xf) + (toAdd & 0xf) + (carry & 0xf) > 0xf)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_H);
	else
		m_RegisterAF.lo = CLEARBIT(m_RegisterAF.lo, FLAG_H);

	if ((initial + toAdd + carry) > 0xFF)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_C);
}

void Emulator::CPU_OR(BYTE val)
{
	m_RegisterAF.hi |= val;

	m_RegisterAF.lo = 0;

	if (m_RegisterAF.hi == 0)
		m_RegisterAF.hi = SETBIT(m_RegisterAF.hi, FLAG_Z);
}

void Emulator::CPU_AND(BYTE val, bool useImmediate)
{
	if (useImmediate)
	{
		val = readMemory(m_ProgramCounter);
		m_ProgramCounter++;
	}

	m_RegisterAF.hi &= val;

	m_RegisterAF.lo = 0;

	if (m_RegisterAF.hi == 0)
		m_RegisterAF.hi = SETBIT(m_RegisterAF.hi, FLAG_Z);

	m_RegisterHL.hi = SETBIT(m_RegisterHL.hi, FLAG_H);
}