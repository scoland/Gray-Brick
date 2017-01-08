#include "stdafx.h"
#include "Emulator.h";

// To use fopen
#pragma warning(disable:4996)

Emulator::Emulator()
{
}

Emulator::~Emulator()
{
}

void Emulator::loadRom()
{
	// sets cartridge memory to 0
	memset(m_CartridgeMemory, 0, sizeof(m_CartridgeMemory));

	// sets in to pointer to file
	FILE * in;

	// open file, flag 'rb' means 'read binary'
	in = fopen("Tetris.gb", "rb");

	// Put rom into memory
	fread(m_CartridgeMemory, 1, 0x200000, in);
	fclose(in);
}

void Emulator::update()
{
	const int MAXCYCLES = 69905;
	int cyclesThisUpdate = 0;

	while (cyclesThisUpdate < MAXCYCLES)
	{
		// ExecuteNextOpcode will return the number of cycles the operation took to execute
		int cycles = ExecuteNextOpcode();
		cyclesThisUpdate += cycles;

		// Timer and graphics are passed the number of cycles the opcode took to execute
		// This is so they can update at the same rate as the CPU
		UpdateTimers(cycles);
		UpdateGraphics(cycles);

		DoInterrupts();
	}

	RenderScreen();

}

void Emulator::resetCPU()
{
	m_ProgramCounter = 0x100;
	m_RegisterAF.reg = 0x01B0;
	m_RegisterBC.reg = 0x0013;
	m_RegisterDE.reg = 0x00D8;
	m_RegisterHL.reg = 0x014D;
	m_StackPointer.reg = 0xFFFE;

	m_Rom[0xFF05] = 0x00;
	m_Rom[0xFF06] = 0x00;
	m_Rom[0xFF07] = 0x00;
	m_Rom[0xFF10] = 0x80;
	m_Rom[0xFF11] = 0xBF;
	m_Rom[0xFF12] = 0xF3;
	m_Rom[0xFF14] = 0xBF;
	m_Rom[0xFF16] = 0x3F;
	m_Rom[0xFF17] = 0x00;
	m_Rom[0xFF19] = 0xBF;
	m_Rom[0xFF1A] = 0x7F;
	m_Rom[0xFF1B] = 0xFF;
	m_Rom[0xFF1C] = 0x9F;
	m_Rom[0xFF1E] = 0xBF;
	m_Rom[0xFF20] = 0xFF;
	m_Rom[0xFF21] = 0x00;
	m_Rom[0xFF22] = 0x00;
	m_Rom[0xFF23] = 0xBF;
	m_Rom[0xFF24] = 0x77;
	m_Rom[0xFF25] = 0xF3;
	m_Rom[0xFF26] = 0xF1;
	m_Rom[0xFF40] = 0x91;
	m_Rom[0xFF42] = 0x00;
	m_Rom[0xFF43] = 0x00;
	m_Rom[0xFF45] = 0x00;
	m_Rom[0xFF47] = 0xFC;
	m_Rom[0xFF48] = 0xFF;
	m_Rom[0xFF49] = 0xFF;
	m_Rom[0xFF4A] = 0x00;
	m_Rom[0xFF4B] = 0x00;
	m_Rom[0xFFFF] = 0x00;

	m_MBC1 = false;
	m_MBC2 = false;

	// Set default clock speed
	m_TimerCounter = 1024;
	m_DividerCounter = 0;

	// Checks to see which MBC type the cartridge is.
	switch (m_CartridgeMemory[0x147])
	{
		case 1:
			m_MBC1 = true;
			break;

		case 2:
			m_MBC1 = true;
			break;

		case 3:
			m_MBC1 = true;
			break;

		case 5:
			m_MBC2 = true;
			break;
		case 6:
			m_MBC2 = true;
			break;

		default:
		break;
	}

	// Sets the current rom bank
	m_CurrentROMBank = 1;

	// Clear out RANBanks
	memset(&m_RAMBanks, 0, sizeof(m_RAMBanks));
	m_CurrentRAMBank = 0;
}

void Emulator::writeMemory(WORD address, BYTE data)
{
	// This is read only
	if (address < 0x8000)
	{
		// This write is an instruction meant for the MBC
		HandleBanking(address, data);
	}
	// Writing to switchable RAM bank only possible when m_EnableRAM is true
	else if (address >= 0xA000 && address <= 0xBFFF)
	{
		if (m_EnableRAM)
		{
			WORD newAddress = address - 0xA000;
			m_RAMBanks[newAddress + (m_CurrentRAMBank * 0x2000)] = data;
		}
	}
	// This is echo memory. Anything written here is also written to 0xC000-0xDDFF region
	else if (address >= 0xE000 && address < 0xFE00)
	{
		m_Rom[address] = data;
		writeMemory(address - 0x2000, data);
	}
	// This is memory location is restricted
	else if (address >= 0xFEA0 && address < 0xFF00)
	{
		printf("Attempt to write memory location %X failed. Restricted memory location.", address);
	}
	else if (address == TMC)
	{
		BYTE currentFreq = getClockFreq();
		m_Rom[TMC] = data;
		BYTE newFreq = getClockFreq();

		if (currentFreq != newFreq)
		{
			setClockFreq();
		}
	}
	// Trap divider register
	else if (address == 0xFF04)
	{
		m_Rom[0xFF04] = 0;
	}
	// Reset current scanline
	else if (address == 0xFF44)
	{
		m_Rom[address] = 0;
	}
	// At this point we can just write the data to memory
	else
	{
		m_Rom[address] = data;
	}
}

// Read memory should never alter/write to the memory, thus it is a const function
BYTE Emulator::readMemory(WORD address) const
{
	// If we're reading from first switchable ROM bank
	// ROM banks are of 0x4000 size
	if (address >= 0x4000 && address <= 0x7FFF)
	{
		// First, we bring the address down 1 bank
		WORD newAddress = address - 0x4000;
		// Then we bring it back up to the correct bank depending on the current ROM bank
		return m_CartridgeMemory[newAddress + (m_CurrentROMBank * 0x4000)];
	}
	// If we're reading from the switchable RAM bank... (This is external on cartridge RAM)
	// RAM banks are of 0x2000 size
	else if (address >= 0xA000 && address <= 0xBFFF)
	{
		WORD newAddress = address - 0xA000;
		return m_RAMBanks[newAddress + (m_CurrentRAMBank * 0x2000)];
	}
	// Otherwise, just return that location in memory
	return m_Rom[address];
}

void Emulator::handleBanking(WORD address, BYTE data)
{
	if (address < 0x2000)
	{
		if (m_MBC1 || m_MBC2)
		{
			RAMBankEnable(address, data);
		}
	}
	// Change the lower 5 bits of ROM bank
	else if ((address >= 0x2000) && (address < 0x4000))
	{
		if (m_MBC1 || m_MBC2)
		{
			changeLoROMBank(data);
		}
	}
	// Either change RAM bank or upper 2 bits of ROM bank
	else if ((address >= 0x4000) && (address < 0x6000))
	{

		if (m_MBC1)
		{
			if (m_ROMBanking)
			{
				changeHiROMBank(data);
			}
			else {
				RAMBankChange(data);
			}
		}
	}
	else if ((address >= 0x6000) && (address <= 0x8000))
	{
		if (m_MBC1)
		{
			changeROMRAMMode(data);
		}
	}
}

void Emulator::ROMBankEnable(WORD address, BYTE data)
{
	// In MBC2 the fourth bit needs to be 0 to enable/disable RAM
	if (m_MBC2)
	{
		if (testBit(address, 4) == 1) return;
	}

	BYTE testData = data & 0xF;
	if (testData == 0xA)
	{
		m_EnableRAM = true;
	}
	else if (testData == 0x0)
	{
		m_EnableRAM = false;
	}
}

void Emulator::changeLoROMBank(BYTE data)
{
	if (m_MBC2)
	{
		data &= 0xF;
		m_CurrentROMBank = data;
		if (m_CurrentROMBank == 0) m_CurrentROMBank++;
	}

	else {
		BYTE lower5 = data & 0x1F;

		// Clear the lower 5 bits
		m_CurrentROMBank &= 0xE0;

		// Insert our new lower5 into our CurrentROMBank
		m_CurrentROMBank |= lower5;

		if (m_CurrentROMBank == 0) m_CurrentROMBank++;
	}
}

void Emulator::changeHiROMBank(BYTE data)
{
	// Clear lower 5 bits of the data
	data = data & 0xE0;

	// Clear upper 3 bits of current ROM
	m_CurrentROMBank &= 0x1F;

	// Insert our new upper3 into our CurrentROMBank
	m_CurrentROMBank |= data;

	if (m_CurrentROMBank == 0) m_CurrentROMBank++;
}

void Emulator::RAMBankChange(BYTE data)
{
	m_CurrentRAMBank = 0x3;
}

void Emulator::changeROMRAMMode(BYTE data)
{
	BYTE ROMOrRAM = data & 0x1;
	m_ROMBanking = (ROMOrRAM == 0) ? true : false;

	// It is important to set m_CurrentRAMBank to 0 whenever you set m_RomBanking to true because the gameboy can only use rambank 0 in this mode.
	if (m_ROMBanking)
		m_CurrentRAMBank = 0;
}

void Emulator::updateTimers(int cycles)
{
	doDividerRegister(cycles);

	// the clock must be enabled to update the clock
	if (isClockEnabled())
	{
		m_TimerCounter -= cycles;

		// enough CPU clock cycles have happened to update the timer
		if (m_TimerCounter <= 0)
		{
			// reset m_TimerTracer to correct value
			setClockFreq();

			// timer about to  overflow
			if (readMemory(TIMA) == 255)
			{
				writeMemory(TIMA, readMemory(TMA));
				requestInterrupt(2);
			}
			else
			{
				writeMemory(TIMA, readMemory(TIMA) + 1);
			}
		}
	}
}

bool Emulator::isClockEnabled() const
{
	// Bit 2 of the TMC is 0 if timer is disabled, and 1 if timer is enabled
	return testBit(readMemory(TMC), 2) ? true : false;
}

BYTE Emulator::getClockFreq() const
{
	// The clock frequency is a combination of bit 0 and 1 of TMC
	return readMemory(TMC) & 0x3;
}

void Emulator::setClockFreq()
{
	BYTE freq = getClockFreq();
	switch (freq)
	{
		case 0: m_TimerCounter = 1024; break; // freq 4096
		case 1: m_TimerCounter = 16; break;// freq 262144
		case 2: m_TimerCounter = 64; break;// freq 65536
		case 3: m_TimerCounter = 256; break;// freq 16382
	}
}

void Emulator::dividerRegister(int cycles)
{
	m_DividerCounter += cycles;
	if (m_DividerCounter >= 255)
	{
		m_DividerCounter = 0;
		m_Rom[0xFF04]++;
	}
}

void Emulator::requestInterrupt(int id)
{
	// Get current interrupt flag from memory
	BYTE interruptFlag = readMemory(0xFF0F);
	// Turn on the correct bit for the interrupt we're requesting
	interruptFlag = SETBIT(interruptFlag, id);
	writeMemory(0xFF0F, interruptFlag);
}

void Emulator::executeInterrupts()
{
	if (m_InterruptMaster == true)
	{
		BYTE interruptFlag = readMemory(0xFF0F);
		BYTE enabled = readMemory(0xFFFF);

		if (interruptFlag > 0)
		{
			for (int i = 0; i < 5; i++)
			{
				// test to see if the i interrupt has a request AND is enabled
				if (ISBITSET(interruptFlag, i) && ISBITSET(enabled, i))
				{
					serviceInterrupt(i);
				}
			}
		}
	}
}

void Emulator::serviceInterrupt(int interrupt)
{
	m_InterruptMaster = false;

	// Shut off the interrupts bit in the request flag
	BYTE interruptFlag = readMemory(0xFF0F);
	interruptFlag = CLEARBIT(interruptFlag, interrupt);
	writeMemory(0xFF0F, interruptFlag);

	// we must save the current execution address by pushing it onto the stack
	pushWordOntoStack(m_ProgramCounter);
	
	switch (interrupt)
	{
		case 0: m_ProgramCounter = 0x40; break;
		case 1: m_ProgramCounter = 0x48; break;
		case 2: m_ProgramCounter = 0x50; break;
		case 4: m_ProgramCounter = 0x60; break;
	}
}

void Emulator::updateGraphics(int cycles)
{
	setLCDStatus();

	if (isLCDEnabled())
		m_ScanlineCounter -= cycles;
	else
		return;

	if (m_ScanlineCounter <= 0)
	{
		// move onto next scanline
		m_Rom[0xFF44]++;
		BYTE currentLine = readMemory(0xFF44);

		// number of cpu cycles to draw one scanline, from pandocs
		m_ScanlineCounter = 456;

		// request V Blank interrupt
		if (currentLine == 144)
			requestInterrupt(0);
		else if (currentLine > 153)
			m_Rom[0xFF44] = 0
		else if (currentLine < 144)
			drawScanLine();
	}
}

void Emulator::setLCDStatus()
{
	BYTE status = readMemory(0xFF41);
	if (isLCDEnabled() == false)
	{
		// set the mode to 1 during lcd disabled and reset scanline
		m_ScanlineCounter = 456;
		m_Rom[0xFF44] = 0;
		status &= 252;
		status = SETBIT(status, 0);
		writeMemory(0xFF41, status);
		return;
	}

	BYTE currentLine = readMemory(0xFF44);
	BYTE currentMode = status & 0x3;

	BYTE mode = 0;
	bool reqInt = false;

	// if we're inside vblank
	if (currentLine >= 144)
	{
		mode = 1;
		status = SETBIT(status, 0);
		status = CLEARBIT(status, 1);
		reqInt = ISBITSET(status, 4);
	}
	// we're not in vblank
	else
	{
		int mode2bounds = 456 - 80;
		int mode3bounds = mode2bounds - 172;
		// mode 2
		if (m_ScanlineCounter >= mode2bounds)
		{
			mode = 2;
			status = SETBIT(status, 1);
			status = CLEARBIT(status, 0);
			reqInt = ISBITSET(status, 5);
		}
		// mode 3
		else if (m_ScanlineCounter >= mode3bounds)
		{
			mode = 3;
			status = SETBIT(status, 1);
			status = SETBIT(status, 0);
		}
		//mode 0
		else
		{
			mode = 0;
			status = CLEARBIT(status, 0);
			status = CLEARBIT(status, 0);
			reqInt = ISBITSET(status, 3);
		}
	}

	// this means we entered a new mode for the first time so we need to request interrupt
	if (reqInt && (mode != currentMode))
		requestInterrupt(1);

	if (ly == readMemory(0xFF45))
	{
		status = SETBIT(status, 2);
		if (ISBITSET(status, 6))
			requestInterrupt(1);
	}
	else
	{
		status = CLEARBIT(status, 2);
	}

	writeMemory(0xFF41, status);
}