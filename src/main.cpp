#include "Core.h"

int main(int argc, char* argv[])
{

	Core* core = new Core;

	int frameDelay = 1000 / 60; // second number is fps
	while (core->running)
	{

		int frameStart = SDL_GetTicks();

		core->render();
		core->listen();

		int frameEnd = SDL_GetTicks() - frameStart;

		if (frameDelay > frameEnd)
			SDL_Delay(frameDelay - frameEnd);

	}
	delete core;


	return 0;
}

// thank you to that cute coworker girl who helped me to re-discover this game in my life.