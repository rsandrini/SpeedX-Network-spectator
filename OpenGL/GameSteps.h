#ifndef GAMESTEPS_H_INCLUDED
#define GAMESTEPS_H_INCLUDED

#include <Windows.h>
#include "SDL.h"
#include "GL/GL.h"
#include "GL/GLU.h"

class GameSteps
{
    public:
        virtual void setup() = 0;

        virtual void processEvents(const SDL_Event& event) = 0;
        virtual void processLogics(float secs) = 0;
        virtual void draw() const = 0;

        virtual bool ended() = 0;

        virtual void teardown() = 0;
        virtual ~GameSteps() {};
};

#endif // GAMESTEPS_H_INCLUDED
