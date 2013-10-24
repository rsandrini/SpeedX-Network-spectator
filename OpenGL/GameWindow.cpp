#include <Windows.h>

#include "GameWindow.h"
#include "SDL.h"
#include "GL/GL.h"
#include "GameSteps.h"

#include <iostream>
#include <stdexcept>

GameWindow* GameWindow::myInstance = NULL;

GameWindow& GameWindow::getInstance()
{
    if (myInstance == NULL)
        myInstance = new GameWindow();

    return *myInstance;
}

void GameWindow::setup(const std::string& caption, int _width, int _height, int _bpp, bool _fullscreen)
{
    width = _width;
    height = _height;
    bpp = _bpp;
    fullscreen = _fullscreen;

    //Inicializamos o subsistema de video e timer.
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) == -1 )
        throw std::runtime_error(std::string("Unable to init SDL.\nReason:") + SDL_GetError());

    //Tentamos criar a janela
    window = SDL_SetVideoMode(width, height, bpp, setupOpenGL());

    //Sem sucesso? Lançamos uma exceção com o erro.
    if (window == NULL)
        throw std::runtime_error(std::string("Unable to create OpenGL window!\nReason:") + SDL_GetError());

    glViewport(0,0, width, height);

    //Ignoramos repetições, caso a tecla se mantenha pressionada
    if (SDL_EnableKeyRepeat(0,0) == -1)
        std::cout << "Warning: Unable to disable key repeat.";

    //Definimos o título da janela
    setCaption(caption);

    //Escondemos o cursor do mouse
    SDL_ShowCursor(false);

    //Configuramos a função de des-inicialização
    atexit (SDL_Quit);
}

void GameWindow::setCaption(const std::string& caption)
{
    SDL_WM_SetCaption(caption.c_str(), NULL);
}

int GameWindow::createFlags()
{
    //Iniciamos com a OpenGL e paleta de hardware
    int flags = SDL_OPENGL | SDL_HWPALETTE;

    if (fullscreen)
        flags |= SDL_FULLSCREEN;

    const SDL_VideoInfo* info = SDL_GetVideoInfo();

    //Criarmos uma superfície de hardware,
    //se este estiver disponível
    if (info->hw_available)
        flags |= SDL_HWSURFACE;
    else
        flags |= SDL_SWSURFACE;

    //Aceleração por hardware?
    if(info -> blit_hw)
        flags |= SDL_HWACCEL;

    return flags;
}

int GameWindow::setupOpenGL()
{
    //Atributos do opengl
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, bpp);
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute( SDL_GL_ACCUM_RED_SIZE, 0);
    SDL_GL_SetAttribute( SDL_GL_ACCUM_GREEN_SIZE, 0);
    SDL_GL_SetAttribute( SDL_GL_ACCUM_BLUE_SIZE, 0);
    SDL_GL_SetAttribute( SDL_GL_ACCUM_ALPHA_SIZE, 0);

    return createFlags();
}

void GameWindow::show(GameSteps* game)
{
    lastTicks = SDL_GetTicks();

    try
    {
        game->setup();
        while (!game->ended())
        {
            Uint32 thisTicks = SDL_GetTicks();
            ticks = thisTicks - lastTicks;
            lastTicks = thisTicks;

            processEvents(game);
			game->processLogics(getSecs());
            game->draw();

            SDL_GL_SwapBuffers();
        }
    }
    catch (std::exception &e)
    {
        std::cout << "Problems while running game logic: " << std::endl << e.what() << std::endl;
        exit(2);
    }

    game->teardown();
    delete game;
}

unsigned GameWindow::getTicks() const
{
    return ticks;
}

float GameWindow::getSecs() const
{
    return ticks / 1000.0f;
}

int GameWindow::getWidth() const
{
    return width;
}

int GameWindow::getHeight() const
{
    return height;
}

float GameWindow::getRatio() const
{
    return static_cast<float>(width) / height;
}


int GameWindow::getBpp() const
{
    return bpp;
}

bool GameWindow::isFullscreen() const
{
    return fullscreen;
}

void GameWindow::processEvents(GameSteps* game)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
        game->processEvents(event);
}

