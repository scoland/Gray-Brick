#include "stdafx.h"
#include "Emulator.h";

#include <iostream>;

void Emulator::NOP()
{
	/* Do nothing! */
}

void Emulator::CPU_CALL(bool useCondition, int flag, bool condition)
{
	// grab immediate WORD value from memory
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

	BYTE n = readMemory(m_ProgramCounter);
	m_ProgramCounter++;

	m_ProgramCounter += n;
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
	reg++;
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

void Emulator::CPU_RETURN()
{
	m_ProgramCounter = popWordOffStack();
}

void Emulator::CPU_STACK_PUSH(WORD val)
{
	pushWordOntoStack(val);
}

void Emulator::CPU_STACK_POP(WORD reg)
{
	reg = popWordOffStack();
}