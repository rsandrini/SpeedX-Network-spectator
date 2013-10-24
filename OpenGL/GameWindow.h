#ifndef GAMEWINDOW_H_INCLUDED
#define GAMEWINDOW_H_INCLUDED

#define GAMEWINDOW GameWindow::getInstance()

#include <string>

#include <Windows.h>
#include "SDL.h"

class GameSteps;

class GameWindow
{
    public:
        static GameWindow& getInstance();

        void setup(const std::string& caption, int _width, int _height, int _bpp=32, bool _fullscreen=false);
        void setCaption(const std::string& caption);
        void show(GameSteps* game);

        unsigned getTicks() const;
        float getSecs() const; //Same of getTicks, but in seconds

        int getWidth() const;
        int getHeight() const;
        float getRatio() const;

        int getBpp() const;

        bool isFullscreen() const;

    private:
        int width;
        int height;
        int bpp;

        bool fullscreen;

        static GameWindow* myInstance;

        SDL_Surface* window;
        Uint32 lastTicks;
        Uint32 ticks;
        //Disallows creation, copy and assignment
        GameWindow(const GameWindow& other);
        GameWindow& operator=(const GameWindow& other);

        GameWindow() : window(NULL), ticks(0) {}

        //Methods
        int createFlags();
        int setupOpenGL();

        void processEvents(GameSteps* game);
};

#endif // GAMEWINDOW_H_INCLUDED
