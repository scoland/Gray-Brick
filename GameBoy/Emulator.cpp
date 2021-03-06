#include "stdafx.h"
#include "Emulator.h";
#include "opcode_names.h";

#include <iostream>;
#include <iomanip>;
#include <string>;

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
	memset(m_Rom, 0, sizeof(m_Rom));
	resetCPU();

	// sets in to pointer to file
	FILE * in;

	// open file, flag 'rb' means 'read binary'
	in = fopen("09.gb", "rb");

	// Put rom into memory
	fread(m_CartridgeMemory, 1, 0x200000, in);
	fclose(in);

	memcpy(&m_Rom[0x0], &m_CartridgeMemory[0], 0x8000); // this is read only and never changes
}

void Emulator::update()
{
	const int MAXCYCLES = 69905;
	int cyclesThisUpdate = 0;

	while (cyclesThisUpdate < MAXCYCLES)
	{
		// ExecuteNextOpcode will return the number of cycles the operation took to execute
		int cycles = executeNextOpcode();
		cyclesThisUpdate += cycles;

		// Timer and graphics are passed the number of cycles the opcode took to execute
		// This is so they can update at the same rate as the CPU
		updateTimers(cycles);
		updateGraphics(cycles);

		executeInterrupts();
	}
	int test;

	// RenderScreen();

}

void Emulator::resetCPU()
{
	m_InterruptMaster = false;
	m_PendingInterruptDisabled = false;
	m_PendingInterruptEnabled = false;

	m_ProgramCounter = 0x100;
	m_RegisterAF.reg = 0x01B0;
	m_RegisterBC.reg = 0x0013;
	m_RegisterDE.reg = 0x00D8;
	m_RegisterHL.reg = 0x014D;
	m_StackPointer.reg = 0xFFFE;

	m_JoypadState = 0xFF;

	m_ScanlineCounter = 456;

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

	// Clear out RAMBanks
	memset(&m_RAMBanks, 0, sizeof(m_RAMBanks));
	m_CurrentRAMBank = 0;
}

void Emulator::writeMemory(WORD address, BYTE data)
{
	// This is read only
	if (address < 0x8000)
	{
		// This write is an instruction meant for the MBC
		handleBanking(address, data);
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
	// Blargg output
	else if (address == 0xFF02 && data == 0x81)
	{
		std::cout << readMemory(0xFF01);
	}
	// Trap divider register
	else if (address == 0xFF04)
	{
		m_Rom[0xFF04] = 0;
	}
	// Reset current scanline
	else if (address == 0xFF44)
	{
		m_Rom[0xFF44] = 0;
	}
	else if (address == 0xFF46)
	{
		DMATransfer(data);
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
	else if (address == 0xFF44)
	{
		return 0;
	}
	else if (address == 0xFF00) {
		return getJoypadState();
	}
	// Otherwise, just return that location in memory
	return m_Rom[address];
}

// Read a WORD from memory, starting at the PC
WORD Emulator::readWord() const
{
	WORD res = readMemory(m_ProgramCounter + 1);
	res = res << 8;
	res |= readMemory(m_ProgramCounter);
	return res;

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

void Emulator::RAMBankEnable(WORD address, BYTE data)
{
	// In MBC2 the fourth bit needs to be 0 to enable/disable RAM
	if (m_MBC2)
	{
		// bit 4 must be zero
		if (ISBITSET(address, 4) == 1) return;
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
	dividerRegister(cycles);

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
	return ISBITSET(readMemory(TMC), 2) ? true : false;
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

void Emulator::keyPressed(int key) {
	bool previouslyUnset = false;

	// If we're setting from 1 to 0 we may have to request an interrupt
	if (ISBITSET(m_JoypadState, key) == false) {
		previouslyUnset = true;
	}

	// A pressed key has state 0, so set key to 0
	m_JoypadState = CLEARBIT(m_JoypadState, key);

	bool button = true;

	// Standard button or d pad?
	if (key > 3) {
		button = true;
	}
	else {
		button = false;
	}

	BYTE keyReq = m_Rom[0xFF00];
	bool shouldRequestInterrupt = false;

	// only request an interrupt if the button just pressed is one the game cares about
	if (button && !ISBITSET(keyReq, 5)) {
		shouldRequestInterrupt = true;
	}
	else if (!button && !ISBITSET(keyReq, 4)) {
		shouldRequestInterrupt = true;
	}

	if (shouldRequestInterrupt && !previouslyUnset) {
		requestInterrupt(4);
	}
}

void Emulator::keyReleased(int key) {
	m_JoypadState = SETBIT(m_JoypadState, key);
}

BYTE Emulator::getJoypadState() const {
	BYTE res = m_Rom[0xFF00];

	// Flip all the bits
	res ^= 0xFF;

	// are we interested in the standard buttons?
	if (!ISBITSET(res, 4))
	{
		BYTE topJoypad = m_JoypadState >> 4;
		topJoypad |= 0xF0; // turn the top 4 bits on
		res &= topJoypad; // show what buttons are pressed
	}
	else if (!ISBITSET(res, 5)) // directional buttons
	{
		BYTE bottomJoypad = m_JoypadState & 0xF;
		bottomJoypad |= 0xF0;
		res &= bottomJoypad;
	}
	return res;
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
			m_Rom[0xFF44] = 0;
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

	// Mode 1 V-Blank
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
		// mode 2 (first 80 cycles) Searching OAM-RAM
		if (m_ScanlineCounter >= mode2bounds)
		{
			mode = 2;
			status = SETBIT(status, 1);
			status = CLEARBIT(status, 0);
			reqInt = ISBITSET(status, 5);
		}
		// mode 3 (172 cycles) Transferring data to LCD drivers
		else if (m_ScanlineCounter >= mode3bounds)
		{
			mode = 3;
			status = SETBIT(status, 1);
			status = SETBIT(status, 0);
		}
		//mode 0 H-Blank (Remaining cycles)
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

	if (currentLine == readMemory(0xFF45))
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

bool Emulator::isLCDEnabled() const
{
	return ISBITSET(readMemory(0xFF40), 7);
}


// OAM DMA. Basically a circuit designed to copy really fast. In this case, copy sprite data really fast.
// Writing to this register launches a DMA transfer from ROM or RAM to OAM memory (sprite attribute table). 
// The written value specifies the transfer source address divided by 100
void Emulator::DMATransfer(BYTE data)
{
	WORD address = data << 8; // source address is data * 100

	// Iterate through length of OAM
	for (int i = 0; i < 0xA0; i++)
	{
		writeMemory(0xFE00 + i, readMemory(address + i));
	}
}

void Emulator::drawScanLine()
{
	BYTE control = readMemory(0xFF40);

	// If BG Display is enabled
	if (ISBITSET(control, 0))
	{
		// renderTiles();
	}

	// If sprites are enabled
	if (ISBITSET(control, 1))
	{
		// renderSprites();
	}
}

/* void Emulator::renderTiles()
{
	// locate tile in memory region 0x8000 - 0x8FFF
	const WORD memoryRegion = 0x8000;
	const int sizeOfTileInMemory = 16;

	WORD tileDataAddress = memoryRegion + (tileIdentifier * sizeOfTileInMemory);

	// locate tile in memory region 0x8800 - 0x97FF
	const WORD memoryRegion = 0x800;
	const int sizeOfTileInMemory = 16;
	const int offset = 128;

	WORD tileDataAddress = memoryRegion + ((tileIdentifier + offset) * sizeOfTileInMemory);

	WORD tileData = 0;
	WORD backgroundMemory = 0;
	bool unsig = true;

	// where to draw the viewbox and the window
	BYTE scrollY = readMemory(0xFF42);
	BYTE scrollX = readMemory(0xFF43);
	BYTE windowY = readMemory(0xFF4A);
	BYTE windowX = readMemory(0xFF4B) - 7; // god only knows why this is - 7 but it is

	BYTE lcdControl = readMemory(0xFF40);

	bool usingWindow = false;

	// is the window display enabled?
	if (ISBITSET(lcdControl, 5))
	{
		// is the current scanline we're drawing within the windows Y pos?
		if (windowY <= readMemory(0xFF44))
			usingWindow = true;
	}
	
	// which tile memory region are we using?
	if (ISBITSET(lcdControl, 4))
	{
		tileData = 0x8000;
	}
	else
	{
		// This is the one that uses signed bytes
		tileData = 0x8800;
		unsig = false;
	}

	if (usingWindow == false)
	{
		// we're drawing the background, so which background memory region are we using?
		if (ISBITSET(lcdControl, 3))
		{
			backgroundMemory = 0x9C00;
		}
		else
		{
			backgroundMemory = 0x9800;
		}
	}
	else
	{
		// we're drawing the window, so which window memory region are we using?
		if (ISBITSET(lcdControl, 6))
			backgroundMemory = 0x9C00;
		else
			backgroundMemory = 0x9800;
	}

	BYTE yPos = 0;
	// yPos is used to calculate which of the 32 vertical tiles the current scanline is drawing
	if (!usingWindow)
		yPos = scrollY + readMemory(0xFF44);
	else
		yPos = readMemory(0xFF44) - windowY;

	// which of the 8 vertical pixels of the current tile is the scanline on?
	WORD tileRow = (((BYTE)(yPos / 8)) * 32);

	// time to start drawing the 160 horizontal pixels for this scanline
	for (int pixel = 0; pixel < 160; pixel++)
	{
		BYTE xPos = pixel + scrollX;

		// translate the current x pos to window space if necessary
		if (usingWindow)
		{
			if (pixel >= windowX)
			{
				xPos = pixel - windowX;
			}
		}
	}

	// which of the 32 horizontal tiles does the xPos fall within?
	WORD tileCol = (xPos / 8);
	SIGNED_WORD  tileNum;

	// get the tile identity number. This can either be signed or unsigned.
	WORD tileAddress = backgroundMemory + tileRow + tileCol;
	if (unsig)
		tileNum = (BYTE)readMemory(tileAddress);
	else
		tileNum = (SIGNED_BYTE)readMemory(tileAddress);

	// deduce where this tile location is in memory.
}

*/

int Emulator::executeNextOpcode() 
{
	int res = 0;
	BYTE opcode = readMemory(m_ProgramCounter);
	m_ProgramCounter++;
	res = executeOpcode(opcode);

	if (m_PendingInterruptDisabled) {
		if (readMemory(m_ProgramCounter - 1) != 0xF3) {
			m_PendingInterruptDisabled = false;
			m_InterruptMaster = false;
		}
	}

	if (m_PendingInterruptEnabled) {
		if (readMemory(m_ProgramCounter - 1) != 0xFB) {
			m_PendingInterruptEnabled = false;
			m_InterruptMaster = true;
		}
	}

	return res;
}

int Emulator::executeOpcode(BYTE opcode)
{
	//if (opcode != 0xcb)
		//std::cout << "A:" << std::setfill('0') << std::setw(4) << m_RegisterAF.reg << " " << debugFlags(m_RegisterAF.lo) << " BC:" << std::setfill('0') << std::setw(4) << m_RegisterBC.reg << " DE:" << std::setfill('0') << std::setw(4) << m_RegisterDE.reg << " HL:" << std::setfill('0') << std::setw(4) << m_RegisterHL.reg << " PC:" << std::setfill('0') << std::setw(4) << (m_ProgramCounter - 1) << " " << opcode_names[opcode] << " : 0x" << std::hex << static_cast<int>(opcode) << std::endl;

	switch (opcode)
	{
	case 0x00: return opcode_00();
	case 0x01: return opcode_01();
	case 0x02: return opcode_02();
	case 0x03: return opcode_03();
	case 0x04: return opcode_04();
	case 0x05: return opcode_05();
	case 0x06: return opcode_06();
	case 0x07: return opcode_07();
	// Put SP at address (nn)
	case 0x08:
	{
		WORD nn = readWord();
		m_ProgramCounter += 2;

		writeMemory(nn, m_StackPointer.lo);
		nn++;
		writeMemory(nn, m_StackPointer.hi);
		return 20;
	}
	case 0x0A: return opcode_0A();
	case 0x0B: return opcode_0B();
	case 0x0C: return opcode_0C();
	case 0x0D: return opcode_0D();
	case 0x0E: return opcode_0E();
	case 0x0F: return opcode_0F();
	case 0x09: return opcode_09();
	case 0x10:
	{
		m_ProgramCounter++;
		return 4;
	}
	case 0x11: return opcode_11();
	case 0x12: return opcode_12();
	case 0x13: return opcode_13();
	case 0x14: return opcode_14();
	case 0x15: return opcode_15();
	case 0x16: return opcode_16();
	case 0x17: return opcode_17();
	case 0x18: return opcode_18();
	case 0x19: return opcode_19();
	case 0x1A: return opcode_1A();
	case 0x1B: return opcode_1B();
	case 0x1C: return opcode_1C();
	case 0x1D: return opcode_1D();
	case 0x1E: return opcode_1E();
	case 0x1F: return opcode_1F();
	case 0x20: return opcode_20();
	case 0x21: return opcode_21();
	case 0x22:
	{
		writeMemory(m_RegisterHL.reg, m_RegisterAF.hi);
		CPU_16BIT_INC(m_RegisterHL.reg);
		return 8;
	}
	case 0x23: return opcode_23();
	case 0x24: return opcode_24();
	case 0x25: return opcode_25();
	case 0x26: return opcode_26();
	case 0x27: return opcode_27();
	case 0x28: return opcode_28();
	case 0x29: return opcode_29();
	case 0x2A: return opcode_2A();
	case 0x2B: return opcode_2B();
	case 0x2C: return opcode_2C();
	case 0x2D: return opcode_2D();
	case 0x2E: return opcode_2E();
	case 0x2F:
	{
		m_RegisterAF.hi ^= 0xFF;

		m_ProgramCounter++;
		return 4;
	}
	case 0x30: return opcode_30();
	case 0x31: return opcode_31();
	case 0x32:
	{
		writeMemory(m_RegisterHL.reg, m_RegisterAF.hi);
		CPU_16BIT_DEC(m_RegisterHL.reg);
		return 8;
	}
	case 0x33: return opcode_33();
	case 0x34: return opcode_34();
	case 0x35: return opcode_35();
	case 0x36:
	{
		BYTE n = readMemory(m_ProgramCounter);
		m_ProgramCounter++;
		writeMemory(m_RegisterHL.reg, n);
		return 12;
	}
	case 0x37: return opcode_37();
	case 0x38: return opcode_38();
	case 0x39: return opcode_39();
	case 0x3A:
	{
		m_RegisterAF.hi = readMemory(m_RegisterHL.reg);
		m_RegisterHL.reg--;
		return 8;
	}
	case 0x3B: return opcode_3B();
	case 0x3C: return opcode_3C();
	case 0x3D: return opcode_3D();
	case 0x3E: return opcode_3E();
	case 0x3F: return opcode_3F();
	case 0x40: return opcode_40();
	case 0x41: return opcode_41();
	case 0x42: return opcode_42();
	case 0x43: return opcode_43();
	case 0x44: return opcode_44();
	case 0x45: return opcode_45();
	case 0x46: return opcode_46();
	case 0x47: return opcode_47();
	case 0x48: return opcode_48();
	case 0x49: return opcode_49();
	case 0x4A: return opcode_4A();
	case 0x4B: return opcode_4B();
	case 0x4C: return opcode_4C();
	case 0x4D: return opcode_4D();
	case 0x4E: return opcode_4E();
	case 0x4F: return opcode_4F();
	case 0x50: return opcode_50();
	case 0x51: return opcode_51();
	case 0x52: return opcode_52();
	case 0x53: return opcode_53();
	case 0x54: return opcode_54();
	case 0x55: return opcode_55();
	case 0x56: return opcode_56();
	case 0x57: return opcode_57();
	case 0x58: return opcode_58();
	case 0x59: return opcode_59();
	case 0x5A: return opcode_5A();
	case 0x5B: return opcode_5B();
	case 0x5C: return opcode_5C();
	case 0x5D: return opcode_5D();
	case 0x5E: return opcode_5E();
	case 0x5F: return opcode_5F();
	case 0x60: return opcode_60();
	case 0x61: return opcode_61();
	case 0x62: return opcode_62();
	case 0x63: return opcode_63();
	case 0x64: return opcode_64();
	case 0x65: return opcode_65();
	case 0x66: return opcode_66();
	case 0x67: return opcode_67();
	case 0x68: return opcode_68();
	case 0x69: return opcode_69();
	case 0x6A: return opcode_6A();
	case 0x6B: return opcode_6B();
	case 0x6C: return opcode_6C();
	case 0x6D: return opcode_6D();
	case 0x6E: return opcode_6E();
	case 0x6F: return opcode_6F();
	case 0x70: return opcode_70();
	case 0x71: return opcode_71();
	case 0x72: return opcode_72();
	case 0x73: return opcode_73();
	case 0x74: return opcode_74();
	case 0x75: return opcode_75();
	case 0x77: return opcode_77();
	case 0x78: return opcode_78();
	case 0x79: return opcode_79();
	case 0x7A: return opcode_7A();
	case 0x7B: return opcode_7B();
	case 0x7C: return opcode_7C();
	case 0x7D: return opcode_7D();
	case 0x7E: return opcode_7E();
	case 0x7F: return opcode_7F();
	case 0x80: return opcode_80();
	case 0x81: return opcode_81();
	case 0x82: return opcode_82();
	case 0x83: return opcode_83();
	case 0x84: return opcode_84();
	case 0x85: return opcode_85();
	case 0x86: return opcode_86();
	case 0x87: return opcode_87();
	case 0x88: return opcode_88();
	case 0x89: return opcode_89();
	case 0x8A: return opcode_8A();
	case 0x8B: return opcode_8B();
	case 0x8C: return opcode_8C();
	case 0x8D: return opcode_8D();
	case 0x8E: return opcode_8E();
	case 0x8F: return opcode_8F();
	case 0x90: return opcode_90();
	case 0x91: return opcode_91();
	case 0x92: return opcode_92();
	case 0x93: return opcode_93();
	case 0x94: return opcode_94();
	case 0x95: return opcode_95();
	case 0x96: return opcode_96();
	case 0x97: return opcode_97();
	case 0x98: return opcode_98();
	case 0x99: return opcode_99();
	case 0x9A: return opcode_9A();
	case 0x9B: return opcode_9B();
	case 0x9C: return opcode_9C();
	case 0x9D: return opcode_9D();
	case 0x9E: return opcode_9E();
	case 0x9F: return opcode_9F();
	case 0xA0: return opcode_A0();
	case 0xA1: return opcode_A1();
	case 0xA2: return opcode_A2();
	case 0xA3: return opcode_A3();
	case 0xA4: return opcode_A4();
	case 0xA5: return opcode_A5();
	case 0xA6: return opcode_A6();
	case 0xA7: return opcode_A7();
	case 0xA8: return opcode_A8();
	case 0xA9: return opcode_A9();
	case 0xAA: return opcode_AA();
	case 0xAB: return opcode_AB();
	case 0xAC: return opcode_AC();
	case 0xAD: return opcode_AD();
	case 0xAE: return opcode_AE();
	case 0xAF: return opcode_AF();
	case 0xB0: return opcode_B0();
	case 0xB1: return opcode_B1();
	case 0xB2: return opcode_B2();
	case 0xB3: return opcode_B3();
	case 0xB4: return opcode_B4();
	case 0xB5: return opcode_B5();
	case 0xB6: return opcode_B6();
	case 0xB7: return opcode_B7();
	case 0xB8: return opcode_B8();
	case 0xB9: return opcode_B9();
	case 0xBA: return opcode_BA();
	case 0xBB: return opcode_BB();
	case 0xBC: return opcode_BC();
	case 0xBD: return opcode_BD();
	case 0xBE: return opcode_BE();
	case 0xBF: return opcode_BF();
	case 0xC0: return opcode_C0();
	case 0xC1: return opcode_C1();
	case 0xC2: return opcode_C2();
	case 0xC3: return opcode_C3();
	case 0xC4: return opcode_C4();
	case 0xC5: return opcode_C5();
	case 0xC6: return opcode_C6();
	case 0xC7: return opcode_C7();
	case 0xC8: return opcode_C8();
	case 0xC9: return opcode_C9();
	case 0xCB: return executeExtendedOpcode();
	case 0xCA: return opcode_CA();
	case 0xCC: return opcode_CC();
	case 0xCE: return opcode_CE();
	case 0xCF: return opcode_CF();
	case 0xCD: return opcode_CD();
	case 0xD0: return opcode_D0();
	case 0xD1: return opcode_D1();
	case 0xD2: return opcode_D2();
	case 0xD4: return opcode_D4();
	case 0xD5: return opcode_D5();
	case 0xD6: return opcode_D6();
	case 0xD7: return opcode_D7();
	case 0xD8: return opcode_D8();
	case 0xD9:
	{
		m_ProgramCounter = popWordOffStack();
		m_InterruptMaster = true;

		return 8;
	}
	case 0xDA: return opcode_DA();
	case 0xDC: return opcode_DC();
	case 0xDD: 
	{
		return 0;
	}
	case 0xDE: return opcode_DE();
	case 0xDF: return opcode_DF();
	case 0xE0: return opcode_E0();
	case 0xE1: return opcode_E1();
	case 0xE2: return opcode_E2();
	case 0xE5: return opcode_E5();
	case 0xE6: return opcode_E6();
	case 0xE7: return opcode_E7();
	case 0xE8: return opcode_E8();
	case 0xE9:
	{
		m_ProgramCounter = m_RegisterHL.reg;
		return 4;
	}
	case 0xEA: return opcode_EA();
	case 0xEE: return opcode_EE();
	case 0xEF: return opcode_EF();
	case 0xF0:
	{
		BYTE n = readMemory(m_ProgramCounter);
		m_ProgramCounter++;

		WORD address = 0xFF00 + n;
		m_RegisterAF.hi = readMemory(address);
		return 12;
	}
	case 0xF1: return opcode_F1();
	case 0xF2: return opcode_F2();
	case 0xF3: return opcode_F3();
	case 0xF5: return opcode_F5();
	case 0xF6: return opcode_F6();
	case 0xF7: return opcode_F7();
	case 0xF8:
	{
		BYTE n = readMemory(m_ProgramCounter);
		m_ProgramCounter++;

		m_RegisterAF.hi = CLEARBIT(m_RegisterAF.hi, FLAG_Z);
		m_RegisterAF.hi = CLEARBIT(m_RegisterAF.hi, FLAG_N);

		m_RegisterHL.reg = (m_StackPointer.reg + n) & 0xFFFF;

		if (n > 0xFFFF)
			m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_C);
		else
			m_RegisterAF.lo = CLEARBIT(m_RegisterAF.lo, FLAG_C);

		if ((m_StackPointer.reg & 0xF) + (n & 0xF) > 0xF)
			m_RegisterAF.lo = SETBIT(m_RegisterAF.lo, FLAG_H);
		else
			m_RegisterAF.lo = CLEARBIT(m_RegisterAF.lo, FLAG_H);

		return 12;
	}
	case 0xF9: return opcode_F9();
	case 0xFA:
	{
		WORD nn = readWord();
		m_ProgramCounter += 2;

		m_RegisterAF.hi = readMemory(nn);
		return 16;
	}
	case 0xFB: return opcode_FB();
	case 0xFE: return opcode_FE();
	case 0xFF: return opcode_FF();

	default:
		printf("Unknown opcode: ", opcode);
		break;
	}

	return 1;
}

void Emulator::pushWordOntoStack(WORD word)
{
	BYTE hi = word >> 8;
	BYTE lo = word & 0xFF;

	// We can only write to memory one byte at a time
	m_StackPointer.reg--;
	writeMemory(m_StackPointer.reg, hi);
	m_StackPointer.reg--;
	writeMemory(m_StackPointer.reg, lo);
}

WORD Emulator::popWordOffStack()
{
	BYTE lo = readMemory(m_StackPointer.reg);
	m_StackPointer.reg++;
	BYTE hi = readMemory(m_StackPointer.reg);
	m_StackPointer.reg++;

	WORD word = hi << 8;
	word += lo;

	return word;
}

int Emulator::executeExtendedOpcode()
{
	BYTE opcode = readMemory(m_ProgramCounter);
	//std::cout << "A:" << std::setfill('0') << std::setw(4) << m_RegisterAF.reg << " " << debugFlags(m_RegisterAF.lo) << " BC:" << std::setfill('0') << std::setw(4) << m_RegisterBC.reg << " DE:" << std::setfill('0') << std::setw(4) << m_RegisterDE.reg << " HL:" << std::setfill('0') << std::setw(4) << m_RegisterHL.reg << " PC:" << std::setfill('0') << std::setw(4) << (m_ProgramCounter - 1) << " " << opcode_names[opcode] << " : 0x" << std::hex << static_cast<int>(opcode) << std::endl;
	m_ProgramCounter++;

	switch (opcode)
	{
	case 0x00: return opcode_CB_00();
	case 0x01: return opcode_CB_01();
	case 0x02: return opcode_CB_02();
	case 0x03: return opcode_CB_03();
	case 0x04: return opcode_CB_04();
	case 0x05: return opcode_CB_05();
	case 0x07: return opcode_CB_07();
	case 0x08: return opcode_CB_08();
	case 0x09: return opcode_CB_09();
	case 0x10: return opcode_CB_10();
	case 0x11: return opcode_CB_11();
	case 0x12: return opcode_CB_12();
	case 0x13: return opcode_CB_13();
	case 0x14: return opcode_CB_14();
	case 0x15: return opcode_CB_15();
	case 0x17: return opcode_CB_17();
	case 0x0A: return opcode_CB_0A();
	case 0x0B: return opcode_CB_0B();
	case 0x0C: return opcode_CB_0C();
	case 0x0D: return opcode_CB_0D();
	case 0x0F: return opcode_CB_0F();
	case 0x18: return opcode_CB_18();
	case 0x19: return opcode_CB_19();
	case 0x1A: return opcode_CB_1A();
	case 0x1B: return opcode_CB_1B();
	case 0x1C: return opcode_CB_1C();
	case 0x1D: return opcode_CB_1D();
	case 0x1E: return opcode_CB_1E();
	case 0x1F: return opcode_CB_1F();
	case 0x20: return opcode_CB_20();
	case 0x21: return opcode_CB_21();
	case 0x22: return opcode_CB_22();
	case 0x23: return opcode_CB_23();
	case 0x24: return opcode_CB_24();
	case 0x25: return opcode_CB_25();
	case 0x27: return opcode_CB_27();
	case 0x28: return opcode_CB_28();
	case 0x29: return opcode_CB_29();
	case 0x2A: return opcode_CB_2A();
	case 0x2B: return opcode_CB_2B();
	case 0x2C: return opcode_CB_2C();
	case 0x2D: return opcode_CB_2D();
	case 0x2F: return opcode_CB_2F();
	case 0x37: return opcode_CB_37();
	case 0x30: return opcode_CB_30();
	case 0x31: return opcode_CB_31();
	case 0x32: return opcode_CB_32();
	case 0x33: return opcode_CB_33();
	case 0x34: return opcode_CB_34();
	case 0x35: return opcode_CB_35();
	case 0x38: return opcode_CB_38();
	case 0x39: return opcode_CB_39();
	case 0x3A: return opcode_CB_3A();
	case 0x3B: return opcode_CB_3B();
	case 0x3C: return opcode_CB_3C();
	case 0x3D: return opcode_CB_3D();
	case 0x3F: return opcode_CB_3F();
	case 0x40: return opcode_CB_40();
	case 0x41: return opcode_CB_41();
	case 0x42: return opcode_CB_42();
	case 0x43: return opcode_CB_43();
	case 0x44: return opcode_CB_44();
	case 0x45: return opcode_CB_45();
	case 0x46: return opcode_CB_46();
	case 0x47: return opcode_CB_47();
	case 0x48: return opcode_CB_48();
	case 0x49: return opcode_CB_49();
	case 0x4A: return opcode_CB_4A();
	case 0x4B: return opcode_CB_4B();
	case 0x4C: return opcode_CB_4C();
	case 0x4D: return opcode_CB_4D();
	case 0x4E: return opcode_CB_4E();
	case 0x4F: return opcode_CB_4F();
	case 0x50: return opcode_CB_50();
	case 0x51: return opcode_CB_51();
	case 0x52: return opcode_CB_52();
	case 0x53: return opcode_CB_53();
	case 0x54: return opcode_CB_54();
	case 0x55: return opcode_CB_55();
	case 0x56: return opcode_CB_56();
	case 0x57: return opcode_CB_57();
	case 0x58: return opcode_CB_58();
	case 0x59: return opcode_CB_59();
	case 0x5A: return opcode_CB_5A();
	case 0x5B: return opcode_CB_5B();
	case 0x5C: return opcode_CB_5C();
	case 0x5D: return opcode_CB_5D();
	case 0x5E: return opcode_CB_5E();
	case 0x5F: return opcode_CB_5F();
	case 0x60: return opcode_CB_60();
	case 0x61: return opcode_CB_61();
	case 0x62: return opcode_CB_62();
	case 0x63: return opcode_CB_63();
	case 0x64: return opcode_CB_64();
	case 0x65: return opcode_CB_65();
	case 0x66: return opcode_CB_66();
	case 0x67: return opcode_CB_67();
	case 0x68: return opcode_CB_68();
	case 0x69: return opcode_CB_69();
	case 0x6A: return opcode_CB_6A();
	case 0x6B: return opcode_CB_6B();
	case 0x6C: return opcode_CB_6C();
	case 0x6D: return opcode_CB_6D();
	case 0x6E: return opcode_CB_6E();
	case 0x6F: return opcode_CB_6F();
	case 0x70: return opcode_CB_70();
	case 0x71: return opcode_CB_71();
	case 0x72: return opcode_CB_72();
	case 0x73: return opcode_CB_73();
	case 0x74: return opcode_CB_74();
	case 0x75: return opcode_CB_75();
	case 0x76: return opcode_CB_76();
	case 0x77: return opcode_CB_77();
	case 0x78: return opcode_CB_78();
	case 0x79: return opcode_CB_79();
	case 0x7A: return opcode_CB_7A();
	case 0x7B: return opcode_CB_7B();
	case 0x7C: return opcode_CB_7C();
	case 0x7D: return opcode_CB_7D();
	case 0x7E: return opcode_CB_7E();
	case 0x7F: return opcode_CB_7F();
	case 0x80: return opcode_CB_80();
	case 0x81: return opcode_CB_81();
	case 0x82: return opcode_CB_82();
	case 0x83: return opcode_CB_83();
	case 0x84: return opcode_CB_84();
	case 0x85: return opcode_CB_85();
	case 0x86: return opcode_CB_86();
	case 0x87: return opcode_CB_87();
	case 0x88: return opcode_CB_88();
	case 0x89: return opcode_CB_89();
	case 0x8A: return opcode_CB_8A();
	case 0x8B: return opcode_CB_8B();
	case 0x8C: return opcode_CB_8C();
	case 0x8D: return opcode_CB_8D();
	case 0x8E: return opcode_CB_8E();
	case 0x8F: return opcode_CB_8F();
	case 0x90: return opcode_CB_90();
	case 0x91: return opcode_CB_91();
	case 0x92: return opcode_CB_92();
	case 0x93: return opcode_CB_93();
	case 0x94: return opcode_CB_94();
	case 0x95: return opcode_CB_95();
	case 0x96: return opcode_CB_96();
	case 0x97: return opcode_CB_97();
	case 0x98: return opcode_CB_98();
	case 0x99: return opcode_CB_99();
	case 0x9A: return opcode_CB_9A();
	case 0x9B: return opcode_CB_9B();
	case 0x9C: return opcode_CB_9C();
	case 0x9D: return opcode_CB_9D();
	case 0x9E: return opcode_CB_9E();
	case 0x9F: return opcode_CB_9F();
	case 0xA0: return opcode_CB_A0();
	case 0xA1: return opcode_CB_A1();
	case 0xA2: return opcode_CB_A2();
	case 0xA3: return opcode_CB_A3();
	case 0xA4: return opcode_CB_A4();
	case 0xA5: return opcode_CB_A5();
	case 0xA6: return opcode_CB_A6();
	case 0xA7: return opcode_CB_A7();
	case 0xA8: return opcode_CB_A8();
	case 0xA9: return opcode_CB_A9();
	case 0xAA: return opcode_CB_AA();
	case 0xAB: return opcode_CB_AB();
	case 0xAC: return opcode_CB_AC();
	case 0xAD: return opcode_CB_AD();
	case 0xAE: return opcode_CB_AE();
	case 0xAF: return opcode_CB_AF();
	case 0xB0: return opcode_CB_B0();
	case 0xB1: return opcode_CB_B1();
	case 0xB2: return opcode_CB_B2();
	case 0xB3: return opcode_CB_B3();
	case 0xB4: return opcode_CB_B4();
	case 0xB5: return opcode_CB_B5();
	case 0xB6: return opcode_CB_B6();
	case 0xB7: return opcode_CB_B7();
	case 0xB8: return opcode_CB_B8();
	case 0xB9: return opcode_CB_B9();
	case 0xBA: return opcode_CB_BA();
	case 0xBB: return opcode_CB_BB();
	case 0xBC: return opcode_CB_BC();
	case 0xBD: return opcode_CB_BD();
	case 0xBE: return opcode_CB_BE();
	case 0xBF: return opcode_CB_BF();
	case 0xC0: return opcode_CB_C0();
	case 0xC1: return opcode_CB_C1();
	case 0xC2: return opcode_CB_C2();
	case 0xC3: return opcode_CB_C3();
	case 0xC4: return opcode_CB_C4();
	case 0xC5: return opcode_CB_C5();
	case 0xC6: return opcode_CB_C6();
	case 0xC7: return opcode_CB_C7();
	case 0xC8: return opcode_CB_C8();
	case 0xC9: return opcode_CB_C9();
	case 0xCA: return opcode_CB_CA();
	case 0xCB: return opcode_CB_CB();
	case 0xCC: return opcode_CB_CC();
	case 0xCD: return opcode_CB_CD();
	case 0xCE: return opcode_CB_CE();
	case 0xCF: return opcode_CB_CF();
	case 0xD0: return opcode_CB_D0();
	case 0xD1: return opcode_CB_D1();
	case 0xD2: return opcode_CB_D2();
	case 0xD3: return opcode_CB_D3();
	case 0xD4: return opcode_CB_D4();
	case 0xD5: return opcode_CB_D5();
	case 0xD6: return opcode_CB_D6();
	case 0xD7: return opcode_CB_D7();
	case 0xD8: return opcode_CB_D8();
	case 0xD9: return opcode_CB_D9();
	case 0xDA: return opcode_CB_DA();
	case 0xDB: return opcode_CB_DB();
	case 0xDC: return opcode_CB_DC();
	case 0xDD: return opcode_CB_DD();
	case 0xDE: return opcode_CB_DE();
	case 0xDF: return opcode_CB_DF();
	case 0xE0: return opcode_CB_E0();
	case 0xE1: return opcode_CB_E1();
	case 0xE2: return opcode_CB_E2();
	case 0xE3: return opcode_CB_E3();
	case 0xE4: return opcode_CB_E4();
	case 0xE5: return opcode_CB_E5();
	case 0xE6: return opcode_CB_E6();
	case 0xE7: return opcode_CB_E7();
	case 0xE8: return opcode_CB_E8();
	case 0xE9: return opcode_CB_E9();
	case 0xEA: return opcode_CB_EA();
	case 0xEB: return opcode_CB_EB();
	case 0xEC: return opcode_CB_EC();
	case 0xED: return opcode_CB_ED();
	case 0xEE: return opcode_CB_EE();
	case 0xEF: return opcode_CB_EF();
	case 0xF0: return opcode_CB_F0();
	case 0xF1: return opcode_CB_F1();
	case 0xF2: return opcode_CB_F2();
	case 0xF3: return opcode_CB_F3();
	case 0xF4: return opcode_CB_F4();
	case 0xF5: return opcode_CB_F5();
	case 0xF6: return opcode_CB_F6();
	case 0xF7: return opcode_CB_F7();
	case 0xF8: return opcode_CB_F8();
	case 0xF9: return opcode_CB_F9();
	case 0xFA: return opcode_CB_FA();
	case 0xFB: return opcode_CB_FB();
	case 0xFC: return opcode_CB_FC();
	case 0xFD: return opcode_CB_FD();
	case 0xFE: return opcode_CB_FE();
	case 0xFF: return opcode_CB_FF();
	default:
		std::cout << "Extended opcode not found : " << std::hex << opcode << std::endl;
		return 0;
	}

}

std::string Emulator::debugFlags(BYTE flagRegister)
{
	std::string outputFlags = "F:";

	if (ISBITSET(flagRegister, FLAG_Z))
		outputFlags += "Z";
	else
		outputFlags += "-";

	if (ISBITSET(flagRegister, FLAG_N))
		outputFlags += "N";
	else
		outputFlags += "-";

	if (ISBITSET(flagRegister, FLAG_H))
		outputFlags += "H";
	else
		outputFlags += "-";

	if (ISBITSET(flagRegister, FLAG_C))
		outputFlags += "C";
	else
		outputFlags += "-";

	return outputFlags;
}