#pragma once
#include <SDL_ttf.h>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <string>
#include "Cell.h"

#define CELLSIZE 25
#define PADDING 50

#define FONT_FILENAME "latinfont.ttf"
#define MESSAGE_WIN "Minefield cleared!"
#define MESSAGE_LOSE "Game over."
#define MESSAGE_PLAYAGAIN "If you'd like to play again, press any key."

#define ROWS 20
#define CELLS_IN_ROWS 20
#define BOMB_FREQUENCY 8 // higher number is less frequent

struct Core
{
	int x; // number of rows
	int y; // number of cells in rows
	int windowWidth;
	int windowHeight;
	int nonBombs;
	int bombFrequency;
	bool running;
	bool gameLost = false;


	std::vector<std::vector<Cell*>> cell; // vector containing all cells

	TTF_Font* font;
	SDL_Renderer* renderer = NULL;
	SDL_Window* window = NULL;
	SDL_Texture* texture[14] = { NULL };

	Core(int = ROWS, int = CELLS_IN_ROWS, int = BOMB_FREQUENCY);
	~Core();
	

	void prepareTextures();
	void initializeCells();
	void layMines();
	void listen();
	void render();
	void checkIfInRect(int, int, bool); // false is right click, true is left
	void revealCell(Cell*);
	void gameOver(bool); // false is defeat, true is victory
	void restart();
	void openRandomCell();
};
