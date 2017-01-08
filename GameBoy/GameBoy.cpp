// GameBoy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Emulator.h"

Emulator * emulator;

int main()
{
	emulator = new Emulator();

	emulator->loadRom();
    return 0;
}

