#pragma once
#include <SDL.h>

struct Cell
{
	SDL_Rect rect = { 0 };

	bool ghost = true; // this makes for a cleaner code, at memory's expense
	bool revealed = false;
	bool bomb = false;
	bool isFlagged = false;

	int amountOfBombs = 0;
	int indexX;
	int indexY;

	
	void reset()
	{
		revealed = bomb = isFlagged = false;
		amountOfBombs = 0;
	}
};