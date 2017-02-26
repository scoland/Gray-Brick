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
		return;
	}
	else if (ISBITSET(m_RegisterAF.lo, flag) == condition)
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

void Emulator::CPU_8BIT_MEMORY_DEC(WORD address)
{
	BYTE initial = readMemory(address);
	BYTE afterDec = initial - 1;
	writeMemory(address, afterDec);

	if (afterDec == 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_Z);
	else
		m_RegisterAF.lo = CLEARBIT(m_RegisterAF.lo, FLAG_Z);

	m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_N);

	if ((initial & 0x0F) == 0)
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

void Emulator::CPU_8BIT_XOR(BYTE& reg, BYTE toXOR, bool useImmediate)
{
	if (!useImmediate)
	{
		reg ^= toXOR;
	}
	else
	{
		BYTE n = readMemory(m_ProgramCounter);
		m_ProgramCounter++;
		reg ^= n;
	}

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

// rotate right through carry
void Emulator::CPU_RRC(BYTE& reg)
{
	bool isCarrySet = ISBITSET(m_RegisterAF.lo, FLAG_C);
	bool isLSBSet = ISBITSET(reg, 0);

	m_RegisterAF.lo = 0;

	reg >>= 1;

	if (isLSBSet)
	{
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_C);
	}

	if (isCarrySet)
		reg = SETBIT(reg, 7);
	
	if (reg == 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_Z);

}

void Emulator::CPU_RRC_MEMORY(WORD address)
{

	BYTE reg = readMemory(address);

	bool isCarrySet = ISBITSET(m_RegisterAF.lo, FLAG_C);
	bool isLSBSet = ISBITSET(reg, 0);

	m_RegisterAF.lo = 0;

	reg >>= 1;

	if (isLSBSet)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_C);

	if (isCarrySet)
		reg = SETBIT(reg, 7);

	if (reg == 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_Z);

	writeMemory(address, reg);
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

void Emulator::CPU_8BIT_OR(BYTE& reg, BYTE toOr, bool useImmediate)
{
	if (!useImmediate)
	{
		reg |= toOr;
	}
	else
	{
		BYTE n = readMemory(m_ProgramCounter);
		m_ProgramCounter++;
		reg |= n;
	}

	m_RegisterAF.lo = 0;

	if (reg == 0)
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

void Emulator::CPU_8BIT_ADD(BYTE& reg, BYTE toAdd, bool useImmediate, bool addCarry)
{
	BYTE initial = reg;
	BYTE adding = 0;

	if (!useImmediate)
	{
		adding = toAdd;
	}
	else 
	{
		BYTE n = readMemory(m_ProgramCounter);
		m_ProgramCounter++;
		adding = n;
	}

	if (addCarry)
	{
		// if the carry flag is set, add 1
		if (ISBITSET(m_RegisterAF.lo, FLAG_C))
			adding++;
	}

	reg += adding;

	// set the flags
	m_RegisterAF.lo = 0;

	if (reg == 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_Z);

	WORD htest = (initial & 0xF);
	htest += (initial & 0xF);

	if (htest > 0xF)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_H);

	if ((initial + initial) > 0xFF)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_C);
}

// this doesn't actually change the register so there is no pass by reference
void Emulator::CPU_CP(BYTE reg, BYTE toCompare, bool useImmediate)
{
	BYTE initial = reg;
	BYTE toSubtract = 0;

	if (!useImmediate)
	{
		toSubtract = toCompare;
	}
	else 
	{
		BYTE n = readMemory(m_ProgramCounter);
		m_ProgramCounter++;
		toSubtract = n;
	}

	reg -= toSubtract;

	m_RegisterAF.lo = 0;

	if (reg == 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_Z);

	m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_N);

	// set if no borrow
	if (initial < toSubtract)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_C);


	SIGNED_WORD htest = initial & 0xF;
	htest -= (toSubtract & 0xF);

	if (htest < 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_H);
}

void Emulator::CPU_SRL(BYTE& reg)
{
	int lsb = ISBITSET(reg, 0);

	reg >>= 1;

	m_RegisterAF.lo = 0;

	if (reg == 0)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_Z);

	if (lsb)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_C);
}

void Emulator::CPU_ADD_HL(WORD toAdd)
{
	WORD initial = m_RegisterHL.reg;

	m_RegisterHL.reg += toAdd;
	
	m_RegisterAF.lo = CLEARBIT(m_RegisterAF.lo, FLAG_N);

	if ((initial + toAdd) > 0xFFFF)
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_C);
	else
		m_RegisterAF.lo = CLEARBIT(m_RegisterAF.lo, FLAG_C);


	if (((initial & 0xFF00) & 0xF) + ((toAdd >> 8) & 0xF))
		m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_H);
	else
		m_RegisterAF.lo = CLEARBIT(m_RegisterAF.lo, FLAG_H);
}