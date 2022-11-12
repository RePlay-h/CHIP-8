#pragma once
#include <iostream>
#include <SDL2/SDL.h>

class Display {
public:
	Display(const char* title, int windowWidth, int windowHeight, int textureWidth, int textureHeigth);
	~Display();
	void Update(void const* pixels, int pitch);
	bool Input(uint8_t* keys);

private:
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;
};