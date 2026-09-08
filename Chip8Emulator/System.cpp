#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>
#include "System.h"


const unsigned int START_ADDRESS = 0x200; // Programs start at memory location 0x200
const uint8_t FONTSET[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Chip8System::initialize() {
	// Clear RAM and state
	std::fill(std::begin(memory), std::end(memory), 0);
	std::fill(std::begin(V), std::end(V), 0);
	std::fill(std::begin(stack), std::end(stack), 0);
	std::fill(std::begin(video), std::end(video), 0);
	std::fill(std::begin(keypad), std::end(keypad), 0);

	// Reset pointers & timers
	pc = START_ADDRESS;
	I = 0;
	sp = 0;
	delayTimer = 0;
	soundTimer = 0;

	// Reload fontset into RAM at 0x50
	for (unsigned int i = 0; i < 80; ++i) {
		memory[0x50 + i] = FONTSET[i];
	}
}

Chip8System::Chip8System() {
	initialize();
}

bool Chip8System::loadRom(const std::string& filename) {

	// Open file in binary mode and move the cursor to the end to get the file size
	std::ifstream romFile(filename, std::ios::binary | std::ios::ate);

	if (!romFile.is_open()) {
		std::cerr << "Failed to open ROM file: " << filename << std::endl;
		return false;
	}
	else {
		// Get the size of the file and allocate a buffer to read it
		std::streampos filesize = romFile.tellg();

		// Check if the ROM file is too large to fit in memory
		if (filesize > 4096 - START_ADDRESS) {
			std::cerr << "ROM file is too large to fit in memory!" << std::endl;
			romFile.close();
			return false;
		}

		char* buffer = new char[filesize];

		// Move the cursor back to the beginning and read the file into the buffer
		romFile.seekg(0, std::ios::beg);
		romFile.read(buffer, filesize);
		romFile.close();

		// Copy the buffer into the Chip8 memory starting at 0x200
		for (unsigned int i = 0; i < filesize; ++i) {
			memory[START_ADDRESS + i] = static_cast<uint8_t>(buffer[i]);
		}

		delete[] buffer;

		return true;
	}
}

void Chip8System::updateTimers() {
	if (delayTimer > 0) {
		--delayTimer;
	}

	if (soundTimer > 0) {
		--soundTimer;
	}
}

uint16_t Chip8System::fetchOpcode() {
	// Each opcode is 2 bytes, so we fetch two consecutive bytes from memory
	uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
	std::cout << "PC: 0x" << std::hex << pc << " | Opcode: 0x" << opcode << "\n";
	pc += 2; // Move the program counter to the next instruction
	
	return opcode;
}

void Chip8System::decodeAndExecute(uint16_t opcode) {

	switch (opcode & 0xF000) {
		case 0x0000:
			if (opcode == 0x00E0) {
				//Clear display
				memset(video, 0, sizeof(video));
			}
			else if (opcode == 0x00EE) {
				//Return from subroutine
				--sp;
				pc = stack[sp];
			}
			break;
		
		case 0x1000:
			//1nnn - JP addr
			//Jump to adress nnn
			//Set pc to nnn
			pc = opcode & 0x0FFF;
			break;

		case 0x6000: {
			//6xkk - LD Vx, byte
			//Set Vx to kk
			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t kk = opcode & 0x00FF;
			V[x] = kk;
			break;
			}

		case 0x7000: {
			//7xkk - ADD Vx, byte
			//Add kk to Vx, then store the result in Vx
			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t kk = opcode & 0x00FF;
			V[x] += kk;
			break;
		}

		case 0xA000:
			// Set I to the address NNN
			I = opcode & 0x0FFF;
			break;

		case 0xD000:
			//Dxyn = DRW Vx, Vy, nibble
			//Display n-byte sprite tarting at memory location I at (Vx, Vy), set VF = collision
			break;
		


	}
}


void Chip8System::cycle() {
	// Fetch
	uint16_t opcode = fetchOpcode();

	// Decode & Execute
	decodeAndExecute(opcode);

	// Update timers
	updateTimers();
}


