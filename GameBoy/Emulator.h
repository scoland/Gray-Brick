#pragma once

#include <string.h>
#include <stdio.h>

typedef unsigned char BYTE;
typedef char SIGNED_BYTE;
typedef unsigned short WORD;
typedef short SIGNED_WORD;

// Each value is the bit in the F register for that specific flag
// Zero Flag
#define FLAG_Z 7;
// Subtract Flag
#define FLAG_N 6;
// Half carry flag
#define FLAG_H 5;
// Carry flag
#define FLAG_C 4;

// Timer counter. When this overflows (bigger than 0xFF) it will be reset to TMA
// and an interrupt will be requested
const BYTE TIMA = 0xFF05;
// Timer Modulo. Data to be loaded when TIMA overflows
const BYTE TMA = 0xFF06;
// Timer Control. Sets the frequency TIMA is counted at
const BYTE TMC = 0xFF07;

// Number of Hz the DMG clock runs at
#define CLOCKSPEED 4194304;

// Some helper functions
// Set a specific bit in a value
#define SETBIT(val, bit) (val | (1 << bit))
// Tests if bit is set
#define ISBITSET(val, bit) (((val >> bit) & 0x01) == 0x01)
// Sets a bit to 0
#define CLEARBIT(val, bit) (val & ~(1 << bit))

class Emulator
{
public:
	Emulator();
	~Emulator();

	void loadRom();
	void update();
private:
	void resetCPU();

	// Memory functions
	BYTE readMemory(WORD address) const;
	void writeMemory(WORD address, BYTE data);
	void handleBanking(WORD address, BYTE data);
	void RAMBankEnable(WORD address, BYTE data);
	void changeLoROMBank(BYTE data);
	void changeHiROMBank(BYTE data);
	void RAMBankChange(BYTE data);
	void changeROMRAMMode(BYTE data);
	WORD readWord() const;

	// Timer
	void updateTimers(int cycles);
	BYTE getClockFreq() const;
	void setClockFreq();
	void dividerRegister(int cycles);
	bool isClockEnabled() const;
	
	// Interrupt
	void requestInterrupt(int id);
	void executeInterrupts();
	void serviceInterrupt(int interrupt);

	// LCD
	void updateGraphics(int cycles);
	void setLCDStatus();
	bool isLCDEnabled() const;
	void DMATransfer(BYTE data);

	// Graphics
	void drawScanLine();
	void renderTiles();

	// CPU
	int executeNextOpcode();
	int executeOpcode(BYTE opcode);
	void CPU_CALL(bool useCondition, int flag, bool condition);
	void CPU_JUMP(bool useCondition, int flag, bool condition);
	void pushWordOntoStack(WORD word);

	BYTE m_CartridgeMemory[0x200000];
	BYTE m_Rom[0x10000];

	// Use a union with an anonymous struct to give us access to individual registers from a pair
	union Register
	{
		WORD reg;
		struct
		{
			BYTE lo;
			BYTE hi;
		};
	};

	Register m_RegisterAF;
	Register m_RegisterBC;
	Register m_RegisterDE;
	Register m_RegisterHL;

	WORD m_ProgramCounter;
	// Some instructions access either the hi/lo values of the SP so we use our register union to facilitate this
	Register m_StackPointer;

	// Booleans for the memory bank controller types. Most games are MBC1.
	bool m_MBC1;
	bool m_MBC2;

	// Which rom bank is currently loaded into memory space 0x4000-0x7FFF
	BYTE m_CurrentROMBank;

	BYTE m_RAMBanks[0x8000];
	BYTE m_CurrentRAMBank;

	// This variable is responsible for telling us how to act when writing to 0x4000-0x6000
	bool m_ROMBanking;

	// Can we write to external RAM?
	bool m_EnableRAM;

	// equal to CLOCKSPEED/frequency
	int m_TimerCounter;
	int m_DividerCounter;

	// IME interrupt master enable flag
	bool m_InterruptMaster;
};