#include "Core.h"

Core::Core(int x, int y, int bombFrequency): x(x), y(y), bombFrequency(bombFrequency)
{
	running = true;

	int SDLinitTest = SDL_Init(SDL_INIT_VIDEO);
	if (SDLinitTest != 0)
	{
		running = false;
		return;
	}

	windowWidth = y * CELLSIZE + PADDING * 2;
	windowHeight = x * CELLSIZE + PADDING * 2;

	window = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, 0);
	renderer = SDL_CreateRenderer(window, -1, 0);

	if (!window || !renderer)
	{
		running = false;
		return;
	}

	if (TTF_Init() == -1)
	{
		running = false;
		return;
	}
	else
	{
		font = TTF_OpenFont(FONT_FILENAME, 20);
	}

	for (int i = 0; i <= x + 1; i++)
	{
		std::vector <Cell*> row;
		for (int j = 0; j <= y + 1; j++)
		{
			row.push_back(new Cell);
		}
		cell.push_back(row);
	}

	prepareTextures();
	initializeCells();
	layMines();
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);

}

Core::~Core()
{
	for (int i = 0; i < cell.size(); i++)
	{
		for (int j = 0; j < cell[i].size(); j++)
		{
			delete cell[i][j];
			cell[i][j] = NULL;
		}
	}

	for (int i = 0; i < 14; i++)
	{
		SDL_DestroyTexture(texture[i]);
		texture[i] = NULL;
	}
	
	TTF_CloseFont(font);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	font = renderer = window = NULL;
}

void Core::prepareTextures()
{
	SDL_Color color = { 255, 255, 255, 255 };
	SDL_Surface* surface = NULL;

	for (int i = 0; i < 9; i++) // create textures numbered 0 through 8
	{
		std::string s = std::to_string(i);
		surface = TTF_RenderText_Blended(font, s.c_str(), color);
		texture[i] = SDL_CreateTextureFromSurface(renderer, surface);

		color.r += 28;	// more bombs around, more red. max bombs around = 8
		color.g += 1;	// color.r maxvalue = 28*9=252. we start at white,
		color.b += 1;	// i.e. invisible zero
		SDL_FreeSurface(surface);
		surface = NULL;
	}


	// bomb texture
	color.r = 255;
	surface = TTF_RenderText_Blended(font, "B", color);
	texture[9] = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	surface = NULL;

	// lose message texture
	surface = TTF_RenderText_Blended(font, MESSAGE_LOSE, color);
	texture[10] = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	surface = NULL;

	// win message texture
	color.r = 78;
	color.g = 228;
	color.b = 78;
	surface = TTF_RenderText_Blended(font, MESSAGE_WIN, color);
	texture[11] = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	surface = NULL;

	// flagged cell texture
	color.r = 255;
	color.g = 0;
	color.b = 0;
	surface = TTF_RenderText_Blended(font, "!", color);
	texture[12] = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	surface = NULL;

	// press any key texture
	color.r = 0;
	surface = TTF_RenderText_Blended(font, MESSAGE_PLAYAGAIN, color);
	texture[13] = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	surface = NULL;

}

void Core::initializeCells() // setup cell coordinates
{

	int currentY = PADDING;
	for (int i = 1; i <= x; i++)
	{
		int currentX = PADDING;
		for (int j = 1; j <= y; j++)
		{
			cell[i][j]->ghost = false;
			cell[i][j]->indexX = i;
			cell[i][j]->indexY = j;
			cell[i][j]->rect.h = cell[i][j]->rect.w = CELLSIZE;
			cell[i][j]->rect.x = currentX;
			cell[i][j]->rect.y = currentY;
			currentX += CELLSIZE;
		}
		currentY += CELLSIZE;
	}

}

void Core::layMines() // literally lays mines, and counts mines around a cell
{
	srand((unsigned)time(NULL));
	nonBombs = 0;

	for (int i = 1; i <= x; i++) // lay mines
	{
		for (int j = 1; j <= y; j++)
		{
			
			int randomNumber = rand() % bombFrequency;
			if (randomNumber == 0)
				cell[i][j]->bomb = true;

			else
				nonBombs++;
		}
	}

	for (int i = 1; i <= x; i++) // count mines around the cell
	{
		for (int j = 1; j <= y; j++)
		{

			if (cell[i-1][j-1]->bomb)		// northwest
				cell[i][j]->amountOfBombs++;

			if (cell[i - 1][j]->bomb)		// north
				cell[i][j]->amountOfBombs++;

			if (cell[i-1][j+1]->bomb)		// northeast
				cell[i][j]->amountOfBombs++;

			if (cell[i][j-1]->bomb)			// west
				cell[i][j]->amountOfBombs++;

			if (cell[i][j]->bomb)			// including the self, because of the 
				cell[i][j]->amountOfBombs++;	// recursion stop condition for bombs

			if (cell[i][j+1]->bomb)			// east
				cell[i][j]->amountOfBombs++;

			if (cell[i+1][j-1]->bomb)		// southwest
				cell[i][j]->amountOfBombs++;

			if (cell[i+1][j]->bomb)			// south
				cell[i][j]->amountOfBombs++;

			if (cell[i+1][j+1]->bomb)		// southeast
				cell[i][j]->amountOfBombs++;

		}
	}

	openRandomCell();
}

void Core::openRandomCell() // opens random cell at the start, with 0 bombs around
{
	srand((unsigned)time(NULL));
	bool foundIt = false;

	while (!foundIt)
	{
		int a = (rand() % x) + 1;
		int b = (rand() % y) + 1;
		if (cell[a][b]->amountOfBombs == 0)
		{
			revealCell(cell[a][b]);
			foundIt = true;
		}
	}
}

void Core::listen() // handle events
{
	SDL_Event evt;
	if (SDL_WaitEvent(&evt))
	{
		switch (evt.type)
		{
		case SDL_QUIT:
			running = false;
			break;

		case SDL_MOUSEBUTTONUP:
			int mx, my;
			SDL_GetMouseState(&mx, &my);

			if (evt.button.button == SDL_BUTTON_LEFT)
				checkIfInRect(mx, my, true);
			else if (evt.button.button == SDL_BUTTON_RIGHT)
				checkIfInRect(mx, my, false);

			break;

		default:
			break;
		}
	}
}

void Core::checkIfInRect(int mx, int my, bool leftMouseButton)	// If you're my future prospective employer, please keep 
{								// in mind that I can easily turn this from O(n) into O(1),
	bool flag = false;					// I promise. I'll show you if you hire me!
	Cell* clickedCell = NULL;				
	
	for (int i = 1; i <= x && flag == false; i++)
	{
		for (int j = 1; j <= y && flag == false; j++)
		{
			// check if clicked in cell
			if (mx > cell[i][j]->rect.x && mx < cell[i][j]->rect.x + cell[i][j]->rect.w)
				if (my > cell[i][j]->rect.y&& my < cell[i][j]->rect.y + cell[i][j]->rect.h)
				{
					flag = true;
					clickedCell = cell[i][j];
				}
		}
	}

	if (clickedCell && leftMouseButton && !clickedCell->isFlagged) // if clicked in cell with LMB & cell isnt flaggee
		revealCell(clickedCell);
	
	else if (clickedCell && !leftMouseButton) // if clicked in cell with RMB
		clickedCell->isFlagged = !clickedCell->isFlagged;
	
	clickedCell = NULL;
	
}

void Core::revealCell(Cell* currentCell)	// reveal cell + if there are 0 bombs around
{						// it, reveal them recursively
	if (currentCell->revealed || currentCell->ghost)
		return;
		

	currentCell->revealed = true;

	if (!currentCell->bomb)
		nonBombs--;

	if (currentCell->amountOfBombs != 0)
		return;

	int i = currentCell->indexX;
	int j = currentCell->indexY;

	revealCell(cell[i-1][j-1]);	// NW
	revealCell(cell[i-1][j]);	// N
	revealCell(cell[i-1][j+1]);	// NE
	revealCell(cell[i][j-1]);	// W
	revealCell(cell[i][j+1]);	// E
	revealCell(cell[i+1][j-1]);	// SW
	revealCell(cell[i+1][j]);	// S
	revealCell(cell[i+1][j+1]);	// SE
	
	currentCell = NULL;
	
}

void Core::render() // render
{
	// draw cells
	for (int i = 1; i <= x; i++)
	{
		for (int j = 1; j <= y; j++)
		{
			SDL_Rect tempRect = cell[i][j]->rect;

			if (cell[i][j]->revealed) // cell is open
			{
				tempRect.x += 7;
				tempRect.y += 1;
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				SDL_RenderFillRect(renderer, &cell[i][j]->rect);
				if (cell[i][j]->bomb) // its a bomb draw B
				{
					tempRect.x -= 1;
					TTF_SizeText(font, "X", &tempRect.w, &tempRect.h); // X is a magic w & h sample
					SDL_RenderCopy(renderer, texture[9], NULL, &tempRect);
					gameLost = true;
				}
				else	// its not a bomb draw a number
				{
					TTF_SizeText(font, "3", &tempRect.w, &tempRect.h); // 3 is a magic w & h sample
					SDL_RenderCopy(renderer, texture[cell[i][j]->amountOfBombs], NULL, &tempRect);
				}
			}
			else if (cell[i][j]->isFlagged) // cell is flagged
			{
				SDL_SetRenderDrawColor(renderer, 30, 144, 255, 255);
				SDL_RenderFillRect(renderer, &tempRect);
				tempRect.x += 10;
				tempRect.y += 1;
				TTF_SizeText(font, "!", &tempRect.w, &tempRect.h);
				SDL_RenderCopy(renderer, texture[12], NULL, &tempRect);
			}
			else // unopened cell
			{
				SDL_SetRenderDrawColor(renderer, 30, 144, 255, 255);
				SDL_RenderFillRect(renderer, &tempRect);
			}
			
			// draw grid around cells
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderDrawRect(renderer, &cell[i][j]->rect);
			
		}
	}

	if (nonBombs == 0)
		gameOver(true);
	if (gameLost)
		gameOver(false);

	SDL_RenderPresent(renderer);
}

void Core::gameOver(bool win)
{
	SDL_Rect rekt;
	
	if (!win) // if lost
	{
		
		TTF_SizeText(font, MESSAGE_LOSE, &rekt.w, &rekt.h);
		rekt.x = (windowWidth / 2) - (rekt.w / 2);
		rekt.y = 20;
		SDL_RenderCopy(renderer, texture[10], NULL, &rekt);
	}
	else // if won
	{
		TTF_SizeText(font, MESSAGE_WIN, &rekt.w, &rekt.h);
		rekt.x = (windowWidth / 2) - (rekt.w / 2);
		rekt.y = 20;
		SDL_RenderCopy(renderer, texture[11], NULL, &rekt);
	}

	TTF_SizeText(font, MESSAGE_PLAYAGAIN, &rekt.w, &rekt.h); // draw play again message
	rekt.x = (windowWidth / 2) - (rekt.w / 2);
	rekt.y = windowHeight - PADDING;
	SDL_RenderCopy(renderer, texture[13], NULL, &rekt);
	SDL_RenderPresent(renderer);

	SDL_Event evt;
	bool restartGame = false;

	while (!restartGame && running) 	// another game loop
	{
		while (SDL_PollEvent(&evt))	// break out of both loops if 
		{				// restart or quit was requested
			
			switch (evt.type)
			{
			case SDL_QUIT:
				running = false;
				break;

			case SDL_KEYDOWN:
				restart();
				restartGame = true;
				break;
			default:
				break;
			}

			if (restartGame && !running)
				break;
		}
	}


}

void Core::restart() // reset, lay mines again
{
	gameLost = false;
	for (int i = 1; i <= x; i++)
		for (int j = 1; j <= y; j++)
			cell[i][j]->reset();

	layMines();
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);
}
