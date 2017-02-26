#pragma once

#include <string.h>
#include <stdio.h>

typedef unsigned char BYTE;
typedef char SIGNED_BYTE;
typedef unsigned short WORD;
typedef short SIGNED_WORD;

// Each value is the bit in the F register for that specific flag
// Zero Flag
const int FLAG_Z = 7;

// Subtract Flag
const int FLAG_N = 6;
// Half carry flag
const int FLAG_H = 5;
// Carry flag
const int FLAG_C = 4;

// Timer counter. When this overflows (bigger than 0xFF) it will be reset to TMA
// and an interrupt will be requested
const BYTE TIMA = 0xFF05;
// Timer Modulo. Data to be loaded when TIMA overflows
const BYTE TMA = 0xFF06;
// Timer Control. Sets the frequency TIMA is counted at
const BYTE TMC = 0xFF07;

// Number of Hz the DMG clock runs at
const int CLOCKSPEED = 4194304;

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
	void CPU_8BIT_LOAD(BYTE& reg);
	void CPU_16BIT_LOAD(WORD& reg);
	void CPU_REG_LOAD(BYTE& reg, BYTE value);
	void CPU_REG_LOAD_ROM(BYTE& reg, WORD address);
	void CPU_8BIT_INC(BYTE& reg);
	void CPU_8BIT_DEC(BYTE& reg);
	void CPU_8BIT_MEMORY_DEC(WORD address);
	void CPU_16BIT_INC(WORD& reg);
	void CPU_16BIT_DEC(WORD& reg);
	void CPU_8BIT_XOR(BYTE& reg, BYTE toXOR, bool useImmediate);
	void CPU_LD_TO_ADDRESS(BYTE& reg);
	void CPU_LDH_INTO_DATA();
	void CPU_JUMP_IMMEDIATE(bool useCondition, int flag, bool condition);
	void CPU_RETURN(bool useCondition, int flag, bool condition);
	void CPU_STACK_PUSH(WORD val);
	void CPU_STACK_POP(WORD& reg);
	void CPU_LD_I(BYTE& reg, WORD& address);
	void CPU_8BIT_SUB(BYTE& reg, BYTE subtracting, bool useImmediate, bool subCarry);
	void CPU_RRC(BYTE& reg);
	void CPU_RRC_MEMORY(WORD address);
	void CPU_ADC(BYTE toAdd);
	void CPU_8BIT_ADD(BYTE& reg, BYTE toAdd, bool useImmediate, bool addCarry);
	void CPU_CP(BYTE reg, BYTE toCompare, bool useImmediate);
	void CPU_ADD_HL(WORD reg);

	void CPU_8BIT_OR(BYTE& reg, BYTE toOr, bool useImmediate);
	void CPU_AND(BYTE val, bool useImmediate);

	void CPU_SRL(BYTE& reg);
	
	BYTE get_byte_from_pc();
	WORD get_word_from_pc();

	void pushWordOntoStack(WORD word);
	WORD popWordOffStack();

	BYTE m_CartridgeMemory[0x200000];
	BYTE m_Rom[0x10000];

	int executeExtendedOpcode();

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

	// No-op
	void NOP();
	
	// Opcodes
	int opcode_00(); int opcode_01(); int opcode_02(); int opcode_03(); int opcode_04(); int opcode_05(); int opcode_06(); int opcode_07(); int opcode_08(); int opcode_09(); int opcode_0A(); int opcode_0B(); int opcode_0C(); int opcode_0D(); int opcode_0E(); int opcode_0F();
	int opcode_10(); int opcode_11(); int opcode_12(); int opcode_13(); int opcode_14(); int opcode_15(); int opcode_16(); int opcode_17(); int opcode_18(); int opcode_19(); int opcode_1A(); int opcode_1B(); int opcode_1C(); int opcode_1D(); int opcode_1E(); int opcode_1F();
	int opcode_20(); int opcode_21(); int opcode_22(); int opcode_23(); int opcode_24(); int opcode_25(); int opcode_26(); int opcode_27(); int opcode_28(); int opcode_29(); int opcode_2A(); int opcode_2B(); int opcode_2C(); int opcode_2D(); int opcode_2E(); int opcode_2F();
	int opcode_30(); int opcode_31(); int opcode_32(); int opcode_33(); int opcode_34(); int opcode_35(); int opcode_36(); int opcode_37(); int opcode_38(); int opcode_39(); int opcode_3A(); int opcode_3B(); int opcode_3C(); int opcode_3D(); int opcode_3E(); int opcode_3F();
	int opcode_40(); int opcode_41(); int opcode_42(); int opcode_43(); int opcode_44(); int opcode_45(); int opcode_46(); int opcode_47(); int opcode_48(); int opcode_49(); int opcode_4A(); int opcode_4B(); int opcode_4C(); int opcode_4D(); int opcode_4E(); int opcode_4F();
	int opcode_50(); int opcode_51(); int opcode_52(); int opcode_53(); int opcode_54(); int opcode_55(); int opcode_56(); int opcode_57(); int opcode_58(); int opcode_59(); int opcode_5A(); int opcode_5B(); int opcode_5C(); int opcode_5D(); int opcode_5E(); int opcode_5F();
	int opcode_60(); int opcode_61(); int opcode_62(); int opcode_63(); int opcode_64(); int opcode_65(); int opcode_66(); int opcode_67(); int opcode_68(); int opcode_69(); int opcode_6A(); int opcode_6B(); int opcode_6C(); int opcode_6D(); int opcode_6E(); int opcode_6F();
	int opcode_70(); int opcode_71(); int opcode_72(); int opcode_73(); int opcode_74(); int opcode_75(); int opcode_76(); int opcode_77(); int opcode_78(); int opcode_79(); int opcode_7A(); int opcode_7B(); int opcode_7C(); int opcode_7D(); int opcode_7E(); int opcode_7F();
	int opcode_80(); int opcode_81(); int opcode_82(); int opcode_83(); int opcode_84(); int opcode_85(); int opcode_86(); int opcode_87(); int opcode_88(); int opcode_89(); int opcode_8A(); int opcode_8B(); int opcode_8C(); int opcode_8D(); int opcode_8E(); int opcode_8F();
	int opcode_90(); int opcode_91(); int opcode_92(); int opcode_93(); int opcode_94(); int opcode_95(); int opcode_96(); int opcode_97(); int opcode_98(); int opcode_99(); int opcode_9A(); int opcode_9B(); int opcode_9C(); int opcode_9D(); int opcode_9E(); int opcode_9F();
	int opcode_A0(); int opcode_A1(); int opcode_A2(); int opcode_A3(); int opcode_A4(); int opcode_A5(); int opcode_A6(); int opcode_A7(); int opcode_A8(); int opcode_A9(); int opcode_AA(); int opcode_AB(); int opcode_AC(); int opcode_AD(); int opcode_AE(); int opcode_AF();
	int opcode_B0(); int opcode_B1(); int opcode_B2(); int opcode_B3(); int opcode_B4(); int opcode_B5(); int opcode_B6(); int opcode_B7(); int opcode_B8(); int opcode_B9(); int opcode_BA(); int opcode_BB(); int opcode_BC(); int opcode_BD(); int opcode_BE(); int opcode_BF();
	int opcode_C0(); int opcode_C1(); int opcode_C2(); int opcode_C3(); int opcode_C4(); int opcode_C5(); int opcode_C6(); int opcode_C7(); int opcode_C8(); int opcode_C9(); int opcode_CA(); int opcode_CB(); int opcode_CC(); int opcode_CD(); int opcode_CE(); int opcode_CF();
	int opcode_D0(); int opcode_D1(); int opcode_D2(); int opcode_D3(); int opcode_D4(); int opcode_D5(); int opcode_D6(); int opcode_D7(); int opcode_D8(); int opcode_D9(); int opcode_DA(); int opcode_DB(); int opcode_DC(); int opcode_DD(); int opcode_DE(); int opcode_DF();
	int opcode_E0(); int opcode_E1(); int opcode_E2(); int opcode_E3(); int opcode_E4(); int opcode_E5(); int opcode_E6(); int opcode_E7(); int opcode_E8(); int opcode_E9(); int opcode_EA(); int opcode_EB(); int opcode_EC(); int opcode_ED(); int opcode_EE(); int opcode_EF();
	int opcode_F0(); int opcode_F1(); int opcode_F2(); int opcode_F3(); int opcode_F4(); int opcode_F5(); int opcode_F6(); int opcode_F7(); int opcode_F8(); int opcode_F9(); int opcode_FA(); int opcode_FB(); int opcode_FC(); int opcode_FD(); int opcode_FE(); int opcode_FF();

	// Extended opcodes
	int opcode_CB_00(); int opcode_CB_01(); int opcode_CB_02(); int opcode_CB_03(); int opcode_CB_04(); int opcode_CB_05(); int opcode_CB_06(); int opcode_CB_07(); int opcode_CB_08(); int opcode_CB_09(); int opcode_CB_0A(); int opcode_CB_0B(); int opcode_CB_0C(); int opcode_CB_0D(); int opcode_CB_0E(); int opcode_CB_0F();
	int opcode_CB_10(); int opcode_CB_11(); int opcode_CB_12(); int opcode_CB_13(); int opcode_CB_14(); int opcode_CB_15(); int opcode_CB_16(); int opcode_CB_17(); int opcode_CB_18(); int opcode_CB_19(); int opcode_CB_1A(); int opcode_CB_1B(); int opcode_CB_1C(); int opcode_CB_1D(); int opcode_CB_1E(); int opcode_CB_1F();
	int opcode_CB_20(); int opcode_CB_21(); int opcode_CB_22(); int opcode_CB_23(); int opcode_CB_24(); int opcode_CB_25(); int opcode_CB_26(); int opcode_CB_27(); int opcode_CB_28(); int opcode_CB_29(); int opcode_CB_2A(); int opcode_CB_2B(); int opcode_CB_2C(); int opcode_CB_2D(); int opcode_CB_2E(); int opcode_CB_2F();
	int opcode_CB_30(); int opcode_CB_31(); int opcode_CB_32(); int opcode_CB_33(); int opcode_CB_34(); int opcode_CB_35(); int opcode_CB_36(); int opcode_CB_37(); int opcode_CB_38(); int opcode_CB_39(); int opcode_CB_3A(); int opcode_CB_3B(); int opcode_CB_3C(); int opcode_CB_3D(); int opcode_CB_3E(); int opcode_CB_3F();
	int opcode_CB_40(); int opcode_CB_41(); int opcode_CB_42(); int opcode_CB_43(); int opcode_CB_44(); int opcode_CB_45(); int opcode_CB_46(); int opcode_CB_47(); int opcode_CB_48(); int opcode_CB_49(); int opcode_CB_4A(); int opcode_CB_4B(); int opcode_CB_4C(); int opcode_CB_4D(); int opcode_CB_4E(); int opcode_CB_4F();
	int opcode_CB_50(); int opcode_CB_51(); int opcode_CB_52(); int opcode_CB_53(); int opcode_CB_54(); int opcode_CB_55(); int opcode_CB_56(); int opcode_CB_57(); int opcode_CB_58(); int opcode_CB_59(); int opcode_CB_5A(); int opcode_CB_5B(); int opcode_CB_5C(); int opcode_CB_5D(); int opcode_CB_5E(); int opcode_CB_5F();
	int opcode_CB_60(); int opcode_CB_61(); int opcode_CB_62(); int opcode_CB_63(); int opcode_CB_64(); int opcode_CB_65(); int opcode_CB_66(); int opcode_CB_67(); int opcode_CB_68(); int opcode_CB_69(); int opcode_CB_6A(); int opcode_CB_6B(); int opcode_CB_6C(); int opcode_CB_6D(); int opcode_CB_6E(); int opcode_CB_6F();
	int opcode_CB_70(); int opcode_CB_71(); int opcode_CB_72(); int opcode_CB_73(); int opcode_CB_74(); int opcode_CB_75(); int opcode_CB_76(); int opcode_CB_77(); int opcode_CB_78(); int opcode_CB_79(); int opcode_CB_7A(); int opcode_CB_7B(); int opcode_CB_7C(); int opcode_CB_7D(); int opcode_CB_7E(); int opcode_CB_7F();
	int opcode_CB_80(); int opcode_CB_81(); int opcode_CB_82(); int opcode_CB_83(); int opcode_CB_84(); int opcode_CB_85(); int opcode_CB_86(); int opcode_CB_87(); int opcode_CB_88(); int opcode_CB_89(); int opcode_CB_8A(); int opcode_CB_8B(); int opcode_CB_8C(); int opcode_CB_8D(); int opcode_CB_8E(); int opcode_CB_8F();
	int opcode_CB_90(); int opcode_CB_91(); int opcode_CB_92(); int opcode_CB_93(); int opcode_CB_94(); int opcode_CB_95(); int opcode_CB_96(); int opcode_CB_97(); int opcode_CB_98(); int opcode_CB_99(); int opcode_CB_9A(); int opcode_CB_9B(); int opcode_CB_9C(); int opcode_CB_9D(); int opcode_CB_9E(); int opcode_CB_9F();
	int opcode_CB_A0(); int opcode_CB_A1(); int opcode_CB_A2(); int opcode_CB_A3(); int opcode_CB_A4(); int opcode_CB_A5(); int opcode_CB_A6(); int opcode_CB_A7(); int opcode_CB_A8(); int opcode_CB_A9(); int opcode_CB_AA(); int opcode_CB_AB(); int opcode_CB_AC(); int opcode_CB_AD(); int opcode_CB_AE(); int opcode_CB_AF();
	int opcode_CB_B0(); int opcode_CB_B1(); int opcode_CB_B2(); int opcode_CB_B3(); int opcode_CB_B4(); int opcode_CB_B5(); int opcode_CB_B6(); int opcode_CB_B7(); int opcode_CB_B8(); int opcode_CB_B9(); int opcode_CB_BA(); int opcode_CB_BB(); int opcode_CB_BC(); int opcode_CB_BD(); int opcode_CB_BE(); int opcode_CB_BF();
	int opcode_CB_C0(); int opcode_CB_C1(); int opcode_CB_C2(); int opcode_CB_C3(); int opcode_CB_C4(); int opcode_CB_C5(); int opcode_CB_C6(); int opcode_CB_C7(); int opcode_CB_C8(); int opcode_CB_C9(); int opcode_CB_CA(); int opcode_CB_CB(); int opcode_CB_CC(); int opcode_CB_CD(); int opcode_CB_CE(); int opcode_CB_CF();
	int opcode_CB_D0(); int opcode_CB_D1(); int opcode_CB_D2(); int opcode_CB_D3(); int opcode_CB_D4(); int opcode_CB_D5(); int opcode_CB_D6(); int opcode_CB_D7(); int opcode_CB_D8(); int opcode_CB_D9(); int opcode_CB_DA(); int opcode_CB_DB(); int opcode_CB_DC(); int opcode_CB_DD(); int opcode_CB_DE(); int opcode_CB_DF();
	int opcode_CB_E0(); int opcode_CB_E1(); int opcode_CB_E2(); int opcode_CB_E3(); int opcode_CB_E4(); int opcode_CB_E5(); int opcode_CB_E6(); int opcode_CB_E7(); int opcode_CB_E8(); int opcode_CB_E9(); int opcode_CB_EA(); int opcode_CB_EB(); int opcode_CB_EC(); int opcode_CB_ED(); int opcode_CB_EE(); int opcode_CB_EF();
	int opcode_CB_F0(); int opcode_CB_F1(); int opcode_CB_F2(); int opcode_CB_F3(); int opcode_CB_F4(); int opcode_CB_F5(); int opcode_CB_F6(); int opcode_CB_F7(); int opcode_CB_F8(); int opcode_CB_F9(); int opcode_CB_FA(); int opcode_CB_FB(); int opcode_CB_FC(); int opcode_CB_FD(); int opcode_CB_FE(); int opcode_CB_FF();
};