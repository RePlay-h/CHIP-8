#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <fstream>
#include <random>
#include <map>
#include <functional>
#include <iostream>
#include <format>


class Chip8Debug;

namespace chip8 {
	const unsigned int FONTSET_START_ADDRESS = 0x50;
	const unsigned int VIDEO_HEIGHT = 32;
	const unsigned int VIDEO_WIDTH = 64;

	inline uint8_t fontset[80] =
	{
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



	class Chip8 {
	public:
		friend class Chip8Debug;

		uint8_t registers[16]{};
		uint8_t memory[4096]{};
		uint16_t index{};
		uint16_t pc{};
		uint16_t stack[16]{};
		uint8_t sp{};
		uint8_t delayTimer{};
		uint8_t soundTimer{};
		uint8_t keypad[16]{};
		uint32_t video[64 * 32]{};
		uint16_t opcode{};

		std::random_device rd;
		std::mt19937 gen{ rd() };
		std::uniform_int_distribution<> randomByte{0, 255U};

		std::map<uint16_t, std::function<void()>> table { // The entire opcode is unique
			{0x0, [this]() {
				
				this->table0[opcode & 0x000Fu]();
			}
			},

			{0x1, [this]() { // 1nnn
				uint16_t address = opcode & 0x0FFFu;
				pc = address;
			}
			},

			{0x2, [this]() { //2nnn
				uint16_t address = opcode & 0x0FFFu;
				stack[sp] = pc;
				++sp;
				pc = address;
			}
			},

			{0x3, [this]() { //3xkk
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t byte = opcode & 0x00FFu;
				pc += 2;
				if(registers[Vx] == byte) {
					pc += 2;
				}
			}
			},

			{0x4, [this]() { //4xkk
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t byte = opcode & 0x00FFu;
				pc += 2;
				if (registers[Vx] != byte)
				{
					pc += 2;
				}
			}
			},

			{0x5, [this]() { //5xy0
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t Vy = (opcode & 0x00F0u) >> 4u;
				pc += 2;
				if (registers[Vx] == registers[Vy])
				{
					pc += 2;
				}
			}
			},

			{0x6, [this]() { //6xkk
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t byte = opcode & 0x00FFu;
				pc += 2;
				registers[Vx] = byte;
			}
			},

			{0x7, [this]() { //7xkk
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t byte = opcode & 0x00FFu;
				pc += 2;
				registers[Vx] += byte;
			}
			}, 

			{0x8, [this]() {
				this->table8[opcode & 0x000Fu]();
			}
			},

			{0x9, [this]() { // 9xy0
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t Vy = (opcode & 0x00F0u) >> 4u;
				pc += 2;
				if (registers[Vx] != registers[Vy])
				{
					pc += 2;
				}
			}
			},

			{0xA, [this]() { //Annn
				uint16_t address = opcode & 0x0FFFu;

				index = address;
				pc += 2;
			}
			},

			{0xB, [this]() { //Bnnn
				uint16_t address = opcode & 0x0FFFu;

				pc = registers[0] + address;
			}
			},

			{0xC,[this]() { //Cxkk
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t byte = opcode & 0x00FFu;
				registers[Vx] = ((uint8_t)randomByte(gen)) & byte;
				pc += 2;
			}
			},

			{0xD, [this]() { //Dxyn
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t Vy = (opcode & 0x00F0u) >> 4u;
				uint8_t height = opcode & 0x000Fu;
				
				uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
				uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;
				
				registers[0xF] = 0;

				for (unsigned int row = 0; row < height; ++row)
				{
					uint8_t spriteByte = memory[index + row];

					for (unsigned int col = 0; col < 8; ++col)
					{
							uint8_t spritePixel = spriteByte & (0x80u >> col);
							uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

						if (spritePixel)
						{
							if (*screenPixel == 0xFFFFFFFF)
							{
								registers[0xF] = 1;
							}

							*screenPixel ^= 0xFFFFFFFF;
						}
					}
				}
				pc += 2;
			}
			},

			{0xE, [this]() {
				this->tableE[opcode & 0x000Fu]();
			}
			},

			{0xF, [this]() {
				this->tableF[opcode & 0x00FFu]();
			}
			}
		};

		std::map<uint16_t, std::function<void()>> table0 { // The first three digits are $00E but the fourth digit is unique
			{0x0, [this]() {
				memset(video, 0, sizeof(video));
				pc += 2;
			}
			},

			{14, [this]() {
				--sp;
				pc = stack[sp];
				pc += 2;
				
			}
			}
		};

		std::map<uint16_t, std::function<void()>> table8{ // The first digit (8) repeats but the last digit is unique
			{0x0, [this]() { //8xy0
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t Vy = (opcode & 0x00F0u) >> 4u;
				registers[Vx] = registers[Vy];
				pc += 2;
			}
			},

			{0x1, [this]() { //8xy1
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t Vy = (opcode & 0x00F0u) >> 4u;

				registers[Vx] |= registers[Vy];
				registers[0xF] = 0;//
				pc += 2;
			}
			},

			{0x2, [this]() { //8xy2
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t Vy = (opcode & 0x00F0u) >> 4u;

				registers[Vx] &= registers[Vy];
				registers[0xF] = 0;//
				pc += 2;
			}
			},

			{0x3, [this]() { //8xy3
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t Vy = (opcode & 0x00F0u) >> 4u;

				registers[Vx] ^= registers[Vy];
				registers[0xF] = 0;//
				pc += 2;
			}
			},

			{0x4, [this]() { //8xy4
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t Vy = (opcode & 0x00F0u) >> 4u;

				uint16_t sum = registers[Vx] + registers[Vy];

				if (sum > 255U)
				{
					registers[0xF] = 1;
				}
				else
				{
					registers[0xF] = 0;
				}

				registers[Vx] = sum & 0xFFu;
				pc += 2;
			}
			},

			{0x5, [this]() { //8xy5
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t Vy = (opcode & 0x00F0u) >> 4u;

				if (registers[Vx] < registers[Vy])//
				{
					registers[0xF] = 1;
				}
				else
				{
					registers[0xF] = 0;
				}

				registers[Vx] -= registers[Vy];
				pc += 2;
			}
			},

			{0x6, [this]() { //8xy6
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;

				registers[0xF] = (registers[Vx] & 0x1u);
				registers[Vx] >>= 1;
				registers[Vx] = (uint8_t)registers[Vx];//
				pc += 2;
			}
			},

			{0x7, [this]() { //8xy7
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t Vy = (opcode & 0x00F0u) >> 4u;

				if (registers[Vy] < registers[Vx])
				{
					registers[0xF] = 1;
				}
				else
				{
					registers[0xF] = 0;
				}

				registers[Vx] = registers[Vy] - registers[Vx];
				pc += 2;
			}
			},

			{0xE, [this]() { //8xyE
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;

				registers[0xF] = (registers[Vx] & 0x80u) >> 7u;
				registers[Vx] *= 2;
				pc += 2;
			}
			}
		};

		std::map<uint16_t, std::function<void()>> tableE{ // The first digit (E) repeats but the last two digits are unique
			{0x1, [this]() { //ExA1
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t key = registers[Vx];
				
				pc += 2;
				if (!keypad[key])
				{
					
					pc += 2;
				}
			}
			},

			{0xE, [this]() { //Ex9E
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t key = registers[Vx];
				
				pc += 2;
				if (keypad[key])
				{
					
					pc += 2;
				}
			}
			}
		};

		std::map<uint16_t, std::function<void()>> tableF{ //The first digit (F) repeats but the last two digits are unique
			{0x07, [this]() { //Fx07
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;

				registers[Vx] = delayTimer;
				pc += 2;
			}
			},

			{0x0A, [this]() { //Fx0A
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				
				for (uint8_t i = 0; i < 16; i++) {
					if (keypad[i] != 0) {
						
						registers[Vx] = i;
						pc += 2;
						return;
					}	
				}
				pc -= 2;
			}
			},

			{0x15, [this]() { //Fx15
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;

				delayTimer = registers[Vx];
				pc += 2;
			}
			},

			{0x18, [this]() { //Fx18
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;

				soundTimer = registers[Vx];
				pc += 2;
			}
			},

			{0x1E, [this]() { //Fx1E
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;

				index += registers[Vx];
				pc += 2;
			}
			},

			{0x29, [this]() { //Fx29
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t digit = registers[Vx];

				index = FONTSET_START_ADDRESS + (5 * digit);
				pc += 2;
			}
			},

			{0x33, [this]() { //Fx33
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;
				uint8_t value = registers[Vx];

				memory[index] = value / 100;
				memory[index + 1] = (value / 10) % 10;
				memory[index + 2] = (value % 100) % 10;

				pc += 2;
			}
			},

			{0x55, [this]() { //Fx55
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;

				for (uint8_t i = 0; i <= Vx; ++i)
				{
					memory[index + i] = registers[i];
				}
				pc += 2;
			}
			},

			{0x65, [this]() { //Fx65
				uint8_t Vx = (opcode & 0x0F00u) >> 8u;

				for (uint8_t i = 0; i <= Vx; ++i)
				{
					registers[i] = memory[index + i];
				}
				pc += 2;
			}
			}
		};


	
		Chip8(const std::string filename);
		void Cycle();
	};
}



class Chip8Debug {
public:
	static void PrintSpecifications(const chip8::Chip8* ptr);
	static void PrintError(const char* type, const char* buffer);
};