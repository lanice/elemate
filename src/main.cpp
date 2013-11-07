#include <iostream>
#include <thread>

#include "game.h"

int main( int argc, char** argv )
{
	Game game;	
	game.start(false); //In separate Thread
	game.end(); // Waits for GameThread

	return 0;
}
