#pragma once
#include <cstdint>
#include <string>

class Chip8System {
private:
	// Memory
	uint8_t memory[4096]{}; //4kB RAM

	// Registers
	uint8_t V[16]{}; // General purpose registers V0 to VF
	uint16_t I{};    // Index register
	uint16_t pc{0x200};   // Program counter -> points to current instruction in memory

	// Stack
	uint16_t stack[16]{};
	uint8_t sp; // Stack pointer

	// Timers
	uint8_t delayTimer{};
	uint8_t soundTimer{};

	// Internal Helpers
	uint16_t fetchOpcode();
	void decodeAndExecute(uint16_t opcode);


public:
	Chip8System();
	~Chip8System() = default;

	bool loadRom(const std::string& filename);
	void cycle();
	void updateTimers();
	void initialize();

	// Display
	uint32_t video[64 * 32]; // 64x32 monochrome display

	// Keypad
	uint8_t keypad[16]; // Hex-based keypad (0x0 to 0xF)
};