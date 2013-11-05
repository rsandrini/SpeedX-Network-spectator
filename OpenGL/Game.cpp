#include "Game.h"
#include "GameWindow.h"
#include "Camera.h"
#include <time.h>
#include "RedeTcp.h"


CCamera objCamera; 
const int sA = 30;
tVector3 buffer[sA]; // The buffer complete for draw the tunnel
tVector3 color[sA];

float lastEnd = -0.5f; // The deepest part of the quad
float end = 0; 
bool left;
bool right;
float rotate;
float cameraSpeed=0.01f;
float view = 0.0f;

//Posição da Luz
float posicao[] = {0.0, 0.0, 0.0, 1.0};
// Speed sum
float speed = 0.00005f;
/* [0]-GameOver [1]-Game Run  [-1]-Pause [2]-Loading Network*/
int gameState; 

float timeToUpdateMap=0;

// Network
RedeTcp* tcp = new RedeTcp();

void Game::setup()
{
	//Define a cor de limpeza da tela
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);*/
	glEnable(GL_SMOOTH);
	glEnable (GL_LINE_SMOOTH);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	//glEnable(GL_BLEND); 
	
	gameState = 2;
	rotate = 0.0f;
	
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
	//GLfloat light_ambient[] = {0, 0, 0, 1.0};
	//glLightfv(GL_LIGHT0, GL_AND_INVERTED, light_ambient);
	
//	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 45);
//	GLfloat spot_direction[] = { 0, 0, -1 };
//	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	//Valor para os componentes ambiente e difuso
	/*
	float ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
	float diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
	float specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
	

	// Tipo de material - reflexão
	glMaterialfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glMaterialfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_LIGHT0, GL_SPECULAR, specular);
	*/
	
	//Posição da Luz
	//glLightfv(GL_LIGHT0, GL_POSITION, posicao);

	glMatrixMode(GL_PROJECTION);
		gluPerspective(45, GAMEWINDOW.getRatio(), 0.1, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	objCamera.Position_Camera(0, -0.5f, 2,  0, 0, 0,  0, 0, 0.5);
	srand(time(0));
	generateMap();

	printf("Aguardando cliente conectar...\n");
	tcp->createServer(8280);
	sendMap(buffer); // Send buffer map
	sendMap(color);  // Send buffer color
	gameState = 1;

}

void Game::sendRotation()
{
	char sendBuff[10];
	float aux;
	aux = rotate * 1000.0f;
	memcpy(sendBuff,&aux,sizeof(float));
	tcp->sendMessage(sendBuff,sizeof(float));
}

void Game::sendMap(tVector3 _buffer[30])
{
	
	int ctrlLoop = 0;
	while (ctrlLoop < 30){
		printf("enviado parte %i", ctrlLoop);
		char sendBuff[255];
		float aux;
		int size = 0;
		// Send Map initial
		for(unsigned short int b = ctrlLoop; b < (ctrlLoop+10); b++) {
			// X
			aux = _buffer[b].x * 1000.0f;
			memcpy(sendBuff+size,&aux,sizeof(float));
			size += sizeof(float);
		
			// Y
			aux = _buffer[b].y * 1000.0f;
			memcpy(sendBuff+size,&aux,sizeof(float));
			size += sizeof(float);
		
			// Z
			aux = _buffer[b].z * 1000.0f;
			memcpy(sendBuff+size,&aux,sizeof(float));
			size += sizeof(float);
		
		}
		tcp->sendMessage(sendBuff,size);

		char receiveBuff[10];
		unsigned short int pos = 0;

		printf("Aguardando resposta cliente...\n");
		while(1) {
			tcp->update();
			if(tcp->receiveMessage(receiveBuff,sizeof(receiveBuff))) {
				break;
			}
		}
		ctrlLoop += 10;
	}
}

void Game::sendNetwork()
{
	sendMap(buffer); // Send buffer map
	sendMap(color);  // Send buffer color

	sendRotation(); // Send a float rotation

}



void Game::processEvents(const SDL_Event& event)
{
    switch (event.type)
    {
        case SDL_QUIT:
            exit = true;
            break;
    }
}

void Game::processLogics(float secs)
{
    //Lemos o estado das teclas
    Uint8* keys = SDL_GetKeyState(NULL);

	if (gameState == 1){

		view = objCamera.mView.z + 1.5;
		objCamera.Move_Camera(cameraSpeed);
		cameraSpeed+=speed;

		rebuildMap();

		// Move light
		//posicao[0] = objCamera.mPos.x;
		//posicao[1] = objCamera.mPos.y;
		posicao[2] = objCamera.mPos.z ;
		glLightfv(GL_LIGHT0, GL_POSITION, posicao);

		if(GetKeyState(VK_LEFT) & 0x80) 
		{	
			rotate -= 3.0f;
			if (rotate < 0)
				rotate = 360.0f;
		}

		if(GetKeyState(VK_RIGHT) & 0x80) 
		{
			rotate += 3.0f;
			if (rotate > 360)
				rotate = 0;
		}

		// Check collision
		colision();

		// Send info to client
		sendNetwork();

	}
	
	if (gameState == -1){
	
		printf("Restart\n");
	}

	if (gameState == 0){
		printf("GameOver");
		
		printf("desconectando\n");
		tcp->disconnect();

		delete(tcp);
		gameState = 3;
	}
	/*
	printf("POS [%.2f %.2f %.2f] VIEW [%.2f %.2f %.2f] UP [%.2f %.2f %.2f] \n",
		    objCamera.mPos.x,  objCamera.mPos.y,  objCamera.mPos.z,	
			objCamera.mView.x, objCamera.mView.y, objCamera.mView.z,	
			objCamera.mUp.x,   objCamera.mUp.y,   objCamera.mUp.z);
	*/
}

void Game::colision(){
	for (int i=0; i<sA; i++){
		
		if (buffer[i].x > 0){
			int local = (int)buffer[i].x;
			// Check if view and obstacle colide
			if (view <= buffer[i].y && view >= buffer[i].z){
				switch(local){
					case 1:
							if ((rotate >= 337.5 && rotate <= 360) || (rotate >= 0 && rotate <= 22.5)) 
								gameState = 0;
					
						break;	
					case 12:						
							if (rotate >= 315 && rotate <= 337.5) 
								gameState = 0;
					
						break;	
					case 11:						
							if (rotate >= 292.5 && rotate <= 315) 
								gameState = 0;
					
						break;	
					case 10:						
							if (rotate >= 247.5 && rotate <= 292.5) 
								gameState = 0;
					
						break;	
					case 9:						
							if (rotate >= 225.5 && rotate <= 247.5) 
								gameState = 0;
					
						break;	
					case 8:						
							if (rotate >= 202.5 && rotate <= 225.5) 
								gameState = 0;
					
						break;	
					case 7:						
							if (rotate >= 157.5 && rotate <= 202.5) 
								gameState = 0;
					
						break;	
					case 6:						
							if (rotate >= 135 && rotate <= 157.5) 
								gameState = 0;
					
						break;	
					case 5:						
							if (rotate >= 102.5 && rotate <= 135.5) 
								gameState = 0;
					
						break;	
					case 4:						
							if (rotate >= 112.5 && rotate <= 67.5) 
								gameState = 0;
					
						break;	
					case 3:						
							if (rotate >= 45 && rotate <= 67.5) 
								gameState = 0;
					
						break;	
					case 2:						
							if (rotate >= 22.5 && rotate <= 45) 
								gameState = 0;
					
						break;	

					// The big cubes
					case 13:						
							if ((rotate >= 337.5 && rotate <= 360) || (rotate >= 0 && rotate <= 22.5)
								|| (rotate >= 157.5 && rotate <= 202.5)) 
								gameState = 0;
					
						break;	
					case 14:						
							if ((rotate >= 67.5 && rotate <= 102.5) || (rotate >= 247.5 && rotate <= 292.5)) 
								gameState = 0;
					
						break;	

				}
			}
		}
	}
}

void Game::draw() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	gluLookAt(objCamera.mPos.x,  objCamera.mPos.y,  objCamera.mPos.z,	
			  objCamera.mView.x, objCamera.mView.y, objCamera.mView.z,	
			  objCamera.mUp.x,   objCamera.mUp.y,   objCamera.mUp.z);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	drawTunnel(false);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawTunnel(true);
}

// Remove the quads out of view and create new's in the finish tunnel
void Game::rebuildMap() const{

	int lastInvalid = 0;
	for (int i=0; i<sA; i++){
		if((objCamera.mPos.z) < buffer[i].z){
			lastInvalid = i;
		}
		else{
			if (lastInvalid > 0)
				break;
		}
	}

	if (lastInvalid > 0){
		int c = 0;
		for (int i=lastInvalid; i<sA; i++){
		
			buffer[c] = buffer[i];
			color[c] = color[i];
			c++;
		}

		for (int i=c; i<sA; i++){
			generateRing(i);
		}
	}
}

void Game::drawObstacle(int position, bool border) const{

	int local = buffer[position].x;
	
	float _end = buffer[position].y;
	float _endLast = buffer[position].z;

	if (border){
		float ambient[] = {0, 0, 0, 1.0};
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ambient);
	}
	else{
		float ambient[] = {color[position].x, color[position].y, color[position].z, 1.0};
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ambient);
	}

	switch (local){
		
		// Cube normal
		case 1:
			// FRONT
			glBegin(GL_POLYGON);
					
				glVertex3f( 0.15, -0.65, _end );
				glVertex3f( 0.15, -0.35, _end );
				glVertex3f( -0.15, -0.35, _end );
				glVertex3f( -0.15, -0.65, _end );
			glEnd();

			// LEFT
			glBegin(GL_POLYGON);
				glVertex3f( -0.15, -0.65, _end );
				glVertex3f( -0.15, -0.65, _endLast );
				glVertex3f( -0.15, -0.35, _endLast );
				glVertex3f( -0.15, -0.35, _end );
			glEnd();

			// RIGHT
			glBegin(GL_POLYGON);
				glVertex3f( 0.15, -0.65, _end );
				glVertex3f( 0.15, -0.65, _endLast );
				glVertex3f( 0.15, -0.35, _endLast );
				glVertex3f( 0.15, -0.35, _end );
			glEnd();

			// TOP
			glBegin(GL_POLYGON);
				glVertex3f( 0.15, -0.35, _end );
				glVertex3f( 0.15, -0.35, _endLast );
				glVertex3f( -0.15, -0.35, _endLast );
				glVertex3f( -0.15, -0.35, _end );
			glEnd();
			break;
		
		case 2:
			glBegin(GL_TRIANGLE_FAN);
				glVertex3f( -0.1, -0.3, _end );
				glVertex3f( -0.15, -0.65, _end );
				glVertex3f( -0.15, -0.65, _endLast );
				glVertex3f( -0.45, -0.45, _endLast );
				glVertex3f( -0.45, -0.45, _end );
				glVertex3f( -0.15, -0.65, _end );
			glEnd();
			break;

		case 3:
			glBegin(GL_TRIANGLE_FAN);
				glVertex3f( 0, 0, _end );
				glVertex3f( -0.45, -0.45, _end );
				glVertex3f( -0.45, -0.45, _endLast );
				glVertex3f( -0.65, -0.15, _endLast );
				glVertex3f( -0.65, -0.15, _end);
				glVertex3f( -0.45, -0.45, _end );
			glEnd();
			break;

		// Cube normal in 270º
		case 4:
			// FRONT
			glBegin(GL_POLYGON);
				glVertex3f( -0.65, -0.15, _end );
				glVertex3f( -0.35, -0.15, _end );
				glVertex3f( -0.35, 0.15, _end );
				glVertex3f( -0.65, 0.15, _end );
			glEnd();
	
			// LEFT
			glBegin(GL_POLYGON);
				glVertex3f( -0.35, 0.15, _end );
				glVertex3f( -0.35, 0.15, _endLast );
				glVertex3f( -0.65, 0.15, _endLast );
				glVertex3f( -0.65, 0.15, _end );
			glEnd();

			// RIGHT
			glBegin(GL_POLYGON);
				glVertex3f( -0.35, -0.15, _end );
				glVertex3f( -0.35, -0.15, _endLast );
				glVertex3f( -0.65, -0.15, _endLast );
				glVertex3f( -0.65, -0.15, _end );
			glEnd();

			// TOP
			glBegin(GL_POLYGON);
				glVertex3f( -0.35, -0.15, _end );
				glVertex3f( -0.35, -0.15, _endLast );
				glVertex3f( -0.35, 0.15, _endLast );
				glVertex3f( -0.35, 0.15, _end );
			glEnd();
			break;

		case 5:
			glBegin(GL_TRIANGLE_FAN);
				glVertex3f( 0, 0, _end );
				glVertex3f( -0.65, 0.15, _end );
				glVertex3f( -0.65, 0.15, _endLast );
				glVertex3f( -0.45, 0.45, _endLast );
				glVertex3f( -0.45, 0.45, _end );
				glVertex3f( -0.65, 0.15, _end );
			glEnd();
			break;

		case 6: 
			glBegin(GL_TRIANGLE_FAN);
				glVertex3f( -0.1, 0.3, _end );
				glVertex3f( -0.45, 0.45, _end );
				glVertex3f( -0.45, 0.45, _endLast );
				glVertex3f( -0.15, 0.65, _endLast );
				glVertex3f( -0.15, 0.65, _end);
				glVertex3f( -0.45, 0.45, _end );
			glEnd();
			break;

		// Cube normal in 180º
		case 7:
			glBegin(GL_POLYGON);
				glVertex3f( -0.15, 0.65, _end );
				glVertex3f( 0.15, 0.65, _end );
				glVertex3f( 0.15, 0.35, _end );
				glVertex3f( -0.15, 0.35, _end );
			glEnd();

			// LEFT
			glBegin(GL_POLYGON);
				glVertex3f( 0.15, 0.35, _end );
				glVertex3f( 0.15, 0.35, _endLast );
				glVertex3f( 0.15, 0.65, _endLast );
				glVertex3f( 0.15, 0.65, _end );
			glEnd();

			// RIGHT
			glBegin(GL_POLYGON);
				glVertex3f( -0.15, 0.35, _end );
				glVertex3f( -0.15, 0.35, _endLast );
				glVertex3f( -0.15, 0.65, _endLast );
				glVertex3f( -0.15, 0.65, _end );
			glEnd();

			// TOP
			glBegin(GL_POLYGON);
				glVertex3f( 0.15, 0.35, _end );
				glVertex3f( 0.15, 0.35, _endLast );
				glVertex3f( -0.15, 0.35, _endLast );
				glVertex3f( -0.15, 0.35, _end );
			glEnd();
			break;

		case 8:
			glBegin(GL_TRIANGLE_FAN);
				glVertex3f( 0, 0, _end );
				glVertex3f( 0.45, 0.45, _end );
				glVertex3f( 0.45, 0.45, _endLast );
				glVertex3f( 0.15, 0.65, _endLast );
				glVertex3f( 0.15, 0.65, _end );
				glVertex3f( 0.45, 0.45, _end );
			glEnd();
			break;

		case 9:
			glBegin(GL_TRIANGLE_FAN);
				glVertex3f( 0.3, 0.1, _end );
				glVertex3f( 0.65, 0.15, _end );
				glVertex3f( 0.65, 0.15, _endLast );
				glVertex3f( 0.45, 0.45, _endLast );
				glVertex3f( 0.45, 0.45, _end);
				glVertex3f( 0.65, 0.15, _end );
			glEnd();
			break;

		// Cube normal in 90º
		case 10:
			// FRONT
			glBegin(GL_POLYGON);
				glVertex3f( 0.65, -0.15, _end );
				glVertex3f( 0.65, 0.15, _end );
				glVertex3f( 0.35, 0.15, _end );
				glVertex3f( 0.35, -0.15, _end );
			glEnd();

			// LEFT
			glBegin(GL_POLYGON);
				glVertex3f( 0.65, -0.15, _end );
				glVertex3f( 0.65, -0.15, _endLast );
				glVertex3f( 0.35, -0.15, _endLast );
				glVertex3f( 0.35, -0.15, _end );
			glEnd();

			// RIGHT
			glBegin(GL_POLYGON);
				glVertex3f( 0.65, 0.15, _end );
				glVertex3f( 0.65, 0.15, _endLast );
				glVertex3f( 0.35, 0.15, _endLast );
				glVertex3f( 0.35, 0.15, _end );
			glEnd();

			// TOP
			glBegin(GL_POLYGON);
				glVertex3f( 0.35, -0.15, _end );
				glVertex3f( 0.35, -0.15, _endLast );
				glVertex3f( 0.35, 0.15, _endLast );
				glVertex3f( 0.35, 0.15, _end );
			glEnd();
			break;

		case 11:
			glBegin(GL_TRIANGLE_FAN);
				glVertex3f( 0, 0, _end );
				glVertex3f( 0.65, -0.15, _end );
				glVertex3f( 0.65, -0.15, _endLast );
				glVertex3f( 0.45, -0.45, _endLast );
				glVertex3f( 0.45, -0.45, _end );
				glVertex3f( 0.65, -0.15, _end );
			glEnd();
			break;

		case 12:
			glBegin(GL_TRIANGLE_FAN);
				glVertex3f( 0.1, -0.3, _end );
				glVertex3f( 0.45, -0.45, _end );
				glVertex3f( 0.45, -0.45, _endLast );
				glVertex3f( 0.15, -0.65, _endLast );
				glVertex3f( 0.15, -0.65, _end);
				glVertex3f( 0.45, -0.45, _end );
			glEnd();
			break;

		// Long normal Vertical
		case 13:

			// FRONT
			glBegin(GL_POLYGON);
				glVertex3f( 0.15, -0.65, _end );
				glVertex3f( 0.15, 0.65, _end );
				glVertex3f( -0.15, 0.65, _end );
				glVertex3f( -0.15, -0.65, _end );
			glEnd();

			// LEFT
			glBegin(GL_POLYGON);
				glVertex3f( -0.15, -0.65, _end );
				glVertex3f( -0.15, -0.65, _endLast );
				glVertex3f( -0.15, 0.65, _endLast );
				glVertex3f( -0.15, 0.65, _end );
			glEnd();

			// RIGHT
			glBegin(GL_POLYGON);
				glVertex3f( 0.15, -0.65, _end );
				glVertex3f( 0.15, -0.65, _endLast );
				glVertex3f( 0.15, 0.65, _endLast );
				glVertex3f( 0.15, 0.65, _end );
			glEnd();
			break;

		// Cube normal horizontal
		case 14:
			// TOP
			glBegin(GL_POLYGON);
				glVertex3f( -0.65, -0.15, _end );
				glVertex3f( 0.65, -0.15, _end );
				glVertex3f( 0.65, 0.15, _end );
				glVertex3f( -0.65, 0.15, _end );
			glEnd();
	
			// LEFT
			glBegin(GL_POLYGON);
				glVertex3f( 0.65, 0.15, _end );
				glVertex3f( 0.65, 0.15, _endLast );
				glVertex3f( -0.65, 0.15, _endLast );
				glVertex3f( -0.65, 0.15, _end );
			glEnd();

			// RIGHT
			glBegin(GL_POLYGON);
				glVertex3f( 0.65, -0.15, _end );
				glVertex3f( 0.65, -0.15, _endLast );
				glVertex3f( -0.65, -0.15, _endLast );
				glVertex3f( -0.65, -0.15, _end );
			glEnd();
			break;
	}
}

void Game::generateMap()  const{

	for (int i=0; i<sA; i++){
		generateRing(i);
	}
}

void Game::generateRing(int i) const{
	color[i].x = (float)rand()/((float)RAND_MAX/1.0f);
	color[i].y = (float)rand()/((float)RAND_MAX/1.0f);
	color[i].z = (float)rand()/((float)RAND_MAX/1.0f);

	if( rand()/(RAND_MAX/10) >= 8 )
		buffer[i].x = (float)rand()/((float)RAND_MAX/15.0f);
	else
		buffer[i].x = 0;

	buffer[i].y = end;
	buffer[i].z = lastEnd;

	end = lastEnd;
	lastEnd -= 0.5f;
}

void Game::drawTunnel(bool border) const
{
	glPushMatrix();

	float _end = 0;
	float _endLast = 0;

	float black[] = {0.0, 0.0, 0.0, 1.0};
	float green[] = {0.0, 1.0, 0.0, 1.0};
	float white[] = {1.0, 1.0, 1.0, 1.0};
	
	/*
	glMaterialfv(GL_FRONT, GL_AMBIENT, green);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glMaterialf(GL_FRONT, GL_SHININESS, 50.0);
	*/

	//glRotatef(20.0f, rotate, 0.0f, 0.0f);
	glRotatef(rotate, 0, 0, 1);
	for(int i=0; i<sA; i++){
		_end = buffer[i].y;
		_endLast = buffer[i].z;

		if (border){
			float ambient[] = {0, 0, 0, 1.0};
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ambient);
		} 
		else{
			float ambient[] = {color[i].x, color[i].y, color[i].z, 1.0};
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, ambient);
			//glColor3f(color[position].x, color[position].y, color[position].z); 
		}

		glBegin(GL_QUAD_STRIP);
			//1
			glVertex3f(0.15f, -0.65f, _end);
			glVertex3f(0.15f, -0.65f, _endLast);

			//2
			glVertex3f(-0.15f, -0.65f, _end);
			glVertex3f(-0.15f, -0.65f, _endLast);

			//3
			glVertex3f(-0.45f, -0.45f, _end);
			glVertex3f(-0.45f, -0.45f, _endLast);

			//4
			glVertex3f(-0.65f, -0.15f, _end);
			glVertex3f(-0.65f, -0.15f, _endLast);

			//5
			glVertex3f(-0.65f, 0.15f, _end);
			glVertex3f(-0.65f, 0.15f, _endLast);

			//6
			glVertex3f(-0.45f, 0.45f, _end);
			glVertex3f(-0.45f, 0.45f, _endLast);

			//7
			glVertex3f(-0.15f, 0.65f, _end);
			glVertex3f(-0.15f, 0.65f, _endLast);

			//8
			glVertex3f(0.15f, 0.65f, _end);
			glVertex3f(0.15f, 0.65f, _endLast);

			//9
			glVertex3f(0.45f, 0.45f, _end);
			glVertex3f(0.45f, 0.45f, _endLast);

			//10
			glVertex3f(0.65f, 0.15f, _end);
			glVertex3f(0.65f, 0.15f, _endLast);

			//11
			glVertex3f(0.65f, -0.15f, _end);
			glVertex3f(0.65f, -0.15f, _endLast);

			//12
			glVertex3f(0.45f, -0.45f,_end);
			glVertex3f(0.45f, -0.45f, _endLast);

			//13
			glVertex3f(0.15f, -0.65f, _end);
			glVertex3f(0.15f, -0.65f, _endLast);
		glEnd();

		if (buffer[i].x > 0)
			drawObstacle(i, border);

	}
	glPopMatrix();
}

void Game::teardown()
{
}