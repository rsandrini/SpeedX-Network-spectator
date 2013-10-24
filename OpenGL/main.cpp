#include "GameWindow.h"
#include "Game.h"

int main(int argc,char* argv[])
{
    GAMEWINDOW.setup("OpenGL Game", 800, 600);
    GAMEWINDOW.show(new Game);
	return 0;
}
