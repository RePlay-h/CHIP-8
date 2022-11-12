#include "Chip8.hpp"


chip8::Chip8::Chip8(const std::string filename)
{
	// set program counter in START_ADDRESS
	pc = 0x200;

	// read commands from file
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	

	try {
		std::streampos size = file.tellg();
		std::unique_ptr<char> buffer{ new char[size] };

		file.seekg(0, std::ios::beg);
		file.read(buffer.get(), size);
		file.close();

		for (int i = 0; i < size; i++) {
			memory[i + 0x200] = buffer.get()[i];
		}
	}
	catch(std::exception &ex) {
		Chip8Debug::PrintError("CHIP-8 CONSTRUCTOR", "File does not exist");
	}
	

	//copy views of symbols in memory
	for(unsigned int i = 0; i < 80; i++) {
		memory[80 + i] = chip8::fontset[i];
	}
}

void chip8::Chip8::Cycle() {
	try {
		opcode = (memory[pc] << 8u) | memory[pc + 1];

		table[(opcode & 0xF000u) >> 12u]();



		if (delayTimer > 0) { delayTimer--; }

		if (soundTimer > 0) { soundTimer--; }
	}
	catch(std::exception &ex) {
		Chip8Debug::PrintError("CHIP-8 CYCLE", "Command does not exist");
	}
}

void Chip8Debug::PrintSpecifications(const chip8::Chip8* ptr) {
	std::cout <<
		std::format(
			"\nV0:{0:#9x} INDEX:{16:#10x} [{20:#06x}]\
		\nV1:{1:#9x} SP:{17:#13x} [{21:#06x}]\
		\nV2:{2:#9x} DelayTimer:{18:#5x} [{22:#06x}]\
		\nV3:{3:#9x} SoundTimer:{19:#5x} [{23:#06x}]\
		\nV4:{4:#9x}		      [{24:#06x}]\
		\nV5:{5:#9x}		      [{25:#06x}]\
		\nV6:{6:#9x}		      [{26:#06x}]\
		\nV7:{7:#9x}		      [{27:#06x}]\
		\nV8:{8:#9x}		      [{28:#06x}]\
		\nV9:{9:#9x}		      [{29:#06x}]\
		\nV10:{10:#8x}		      [{30:#06x}]\
		\nV11:{11:#8x}		      [{31:#06x}]\
		\nV12:{12:#8x}		      [{32:#06x}]\
		\nV13:{13:#8x}		      [{33:#06x}]\
		\nV14:{14:#8x}		      [{34:#06x}]\
	    \nV15:{15:#8x}		      [{35:#06x}]\n\n\n\n\n\n\n\n\n\n\n\n\n\n",
			ptr->registers[0], ptr->registers[1], ptr->registers[2],
			ptr->registers[3], ptr->registers[4], ptr->registers[5],
			ptr->registers[6], ptr->registers[7], ptr->registers[8],
			ptr->registers[9], ptr->registers[10], ptr->registers[11],
			ptr->registers[12], ptr->registers[13], ptr->registers[14],
			ptr->registers[15], ptr->index, ptr->sp, ptr->delayTimer,
			ptr->soundTimer, ptr->stack[0], ptr->stack[1], ptr->stack[2],
			ptr->stack[3], ptr->stack[4], ptr->stack[5], ptr->stack[6],
			ptr->stack[7], ptr->stack[8], ptr->stack[9], ptr->stack[10],
			ptr->stack[11], ptr->stack[12], ptr->stack[13], ptr->stack[14],
			ptr->stack[15]);
}

void Chip8Debug::PrintError(const char* type, const char* buffer) {
	std::cerr << "[" << type << "]" << " " << buffer << '\n';
	abort();
}
