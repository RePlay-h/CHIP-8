#include "Chip8/Chip8.hpp"
#include "Chip8/Display.hpp"
#include <chrono>

int main(int argc, char* argv[]) {
	
		if(argc < 4) {
			std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
			return EXIT_FAILURE;
		}
		int videoScale = std::atoi(argv[1]);
		int cycleDelay = std::atoi(argv[2]);
		std::string const filename = argv[3];

		Display dis(
			"CHIP-8 Emulator", chip8::VIDEO_WIDTH * videoScale, chip8::VIDEO_HEIGHT * videoScale, chip8::VIDEO_WIDTH, chip8::VIDEO_HEIGHT);

		chip8::Chip8 chip8{ filename };

		int videoPitch = sizeof(chip8.video[0]) * chip8::VIDEO_WIDTH;
		auto last = std::chrono::high_resolution_clock::now();
		bool quit = false;
		
		while(!quit) {
			quit = dis.Input(chip8.keypad);

			auto current = std::chrono::high_resolution_clock::now();
			float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(current - last).count();
		
			if (dt > cycleDelay)
			{
				last = current;
				
				chip8.Cycle();
				dis.Update(chip8.video, videoPitch);
			}
			
			Chip8Debug::PrintSpecifications(&chip8); 
		}
		return 0;
}
