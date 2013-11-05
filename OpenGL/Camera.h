#pragma once
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glaux.lib")


#include <windows.h>					// Header File For Windows
#include <gl\gl.h>						// Header File For The OpenGL32 Library
#include <gl\glu.h>						// Header File For The GLU32 Library
//#include <gl\glaux.h>					// Header File For The Glaux Library
#include "tVector.h"
#include <gl\GLAux.h>

#define CAMERASPEED	0.01f				// The Camera Speed

class CCamera 
{
	public:

		tVector3 mPos;		// camera position
		tVector3 mView;		// camera view(target)
		tVector3 mUp;		// camera upvector(tilt)

		// This function will move the camera depending on the camera speed
		void Move_Camera(float speed);
	
		// This function sets the camera position
		void Position_Camera(float pos_x,  float pos_y,  float pos_z,
			 				 float view_x, float view_y, float view_z,
							 float up_x,   float up_y,   float up_z);
};

