#ifndef ROTATINGTRIANGLE_H_INCLUDED
#define ROTATINGTRIANGLE_H_INCLUDED
#include "GameSteps.h"
#include "tVector.h"

class Game : public GameSteps
{
    private:
        bool exit;

    public:
		explicit Game() : exit(false) {}

        virtual void setup();

        virtual void processEvents(const SDL_Event& event);
        virtual void processLogics(float secs);
        virtual void draw() const;
		virtual bool ended() { return exit; }
        virtual void teardown();
		void drawTunnel(bool border) const;
		void generateMap() const;
		void rebuildMap() const;
		void drawObstacle(int position, bool border) const;
		void generateRing(int i) const;
		void colision();
		void sendNetwork();
		void sendRotation();
		void sendMap(tVector3 _buffer[30]);
};

#endif // ROTATINGTRIANGLE_H_INCLUDED
