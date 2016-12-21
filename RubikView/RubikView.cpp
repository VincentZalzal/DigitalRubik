#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include "../Cube/cube.h"

struct SGLState
{
	GLfloat m_RotAngle;

	SGLState();
};

SGLState::SGLState()
{
	// Cube initialization
	Cube::Reset();
	m_RotAngle = 0.0f;
}

// Resize And Initialize The GL Window
GLvoid ReSizeGLScene(int width, int height)
{
	if (height == 0)			// Prevent A Divide By Zero By
		height = 1;			// Making Height Equal One

	glViewport(0, 0, width, height);	// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);		// Select The Projection Matrix
	glLoadIdentity();			// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat) width / (GLfloat) height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);		// Select The Modelview Matrix
	glLoadIdentity();			// Reset The Modelview Matrix
}

// All Setup For OpenGL Goes Here
void InitGL()
{
	glShadeModel(GL_SMOOTH);		// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);	// Black Background
	glClearDepth(1.0f);			// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);		// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);			// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
}

void DrawFace(GLfloat px, GLfloat py, GLfloat pz, // first corner
	      GLfloat ux, GLfloat uy, GLfloat uz, // first  vector for 1 facelet
	      GLfloat vx, GLfloat vy, GLfloat vz, // second vector for 1 facelet
	      const uint8_t* FaceletIndices)
{
	const GLfloat Size   = 0.8f;   // percentage of vector that really applies to the facelet
	const GLfloat Delta  = 0.999f; // percentage to reduce inner black cube
	const GLfloat Offset = 0.5f * (1.0f - Size); // percentage of vector for the initial facelet offset

	// Draw inner black face; this is a trick to have a black outline.
	// Use Delta to shrink it just a little.
	glColor3f(0.0f, 0.0f, 0.0f);
	GLfloat qx = px, qy = py, qz = pz;
	glVertex3f(Delta*qx, Delta*qy, Delta*qz);
	qx += 3.0f*ux; qy += 3.0f*uy; qz += 3.0f*uz;
	glVertex3f(Delta*qx, Delta*qy, Delta*qz);
	qx += 3.0f*vx; qy += 3.0f*vy; qz += 3.0f*vz;
	glVertex3f(Delta*qx, Delta*qy, Delta*qz);
	qx -= 3.0f*ux; qy -= 3.0f*uy; qz -= 3.0f*uz;
	glVertex3f(Delta*qx, Delta*qy, Delta*qz);

	// Draw the 9 facelets
	const Facelet::Type* Facelets = Cube::GetFacelets();
	for (int i = 0; i < 3; ++i)
	{
		// Offset is used to skip the black outline only once.
		GLfloat ss = i + Offset;
		GLfloat sx = px + ss * vx;
		GLfloat sy = py + ss * vy;
		GLfloat sz = pz + ss * vz;

		for (int j = 0; j < 3; ++j)
		{
			// Get Color.
			uint8_t FaceletIndex = FaceletIndices[i * 3 + j];
			Facelet::Type FaceletState = Facelets[FaceletIndex];
			SColor Color = Colors[FaceletState];
			glColor3ub(Color.r, Color.g, Color.b);

			// Offset is used to skip the black outline only once.
			GLfloat tt = j + Offset;
			GLfloat tx = sx + tt * ux;
			GLfloat ty = sy + tt * uy;
			GLfloat tz = sz + tt * uz;

			// Use Size to shrink the vector a little for the black outline.
			glVertex3f(tx, ty, tz);
			tx += Size * ux; ty += Size * uy; tz += Size * uz;
			glVertex3f(tx, ty, tz);
			tx += Size * vx; ty += Size * vy; tz += Size * vz;
			glVertex3f(tx, ty, tz);
			tx -= Size * ux; ty -= Size * uy; tz -= Size * uz;
			glVertex3f(tx, ty, tz);
		}
	}
}

// Here's Where We Do All The Drawing
void DrawGLScene(GLFWwindow* Window)
{
	SGLState* pGLState = static_cast<SGLState*>(glfwGetWindowUserPointer(Window));

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();					// Reset The Current Modelview Matrix

	glTranslatef(0.0f, 0.0f, -21.0f);			// Translate Into The Screen 7.0 Units
	//glRotatef(pGLState->m_RotAngle, 0.0f, 1.0f, 0.0f);		// Rotate The cube around the Y axis
	//glRotatef(pGLState->m_RotAngle, 1.0f, 1.0f, 1.0f);
	glRotatef(-30.0f, 0.0f, 1.0f, 0.0f);
	glRotatef( 30.0f, 1.0f, 0.0f, 0.0f);

	glBegin(GL_QUADS);

		// Top
		const uint8_t TopFacelets[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
		DrawFace(-3.0f,  3.0f, -3.0f,
			  0.0f,  0.0f,  2.0f,
			  2.0f,  0.0f,  0.0f,
			  TopFacelets);

		// Front
		const uint8_t FrontFacelets[9] = {9, 10, 11, 12, 13, 14, 15, 16, 17};
		DrawFace(-3.0f,  3.0f,  3.0f,
			  0.0f, -2.0f,  0.0f,
			  2.0f,  0.0f,  0.0f,
			  FrontFacelets);

		// Right
		const uint8_t RightFacelets[9] = {18, 19, 20, 21, 22, 23, 24, 25, 26};
		DrawFace( 3.0f,  3.0f,  3.0f,
			  0.0f, -2.0f,  0.0f,
			  0.0f,  0.0f, -2.0f,
			  RightFacelets);

		// Back
		const uint8_t BackFacelets[9] = {27, 28, 29, 30, 31, 32, 33, 34, 35};
		DrawFace( 3.0f,  3.0f, -3.0f,
			  0.0f, -2.0f,  0.0f,
			 -2.0f,  0.0f,  0.0f,
			  BackFacelets);

		// Left
		const uint8_t LeftFacelets[9] = {36, 37, 38, 39, 40, 41, 42, 43, 44};
		DrawFace(-3.0f,  3.0f, -3.0f,
			  0.0f, -2.0f,  0.0f,
			  0.0f,  0.0f,  2.0f,
			  LeftFacelets);

		// Bottom
		const uint8_t BottomFacelets[9] = {45, 46, 47, 48, 49, 50, 51, 52, 53};
		DrawFace(-3.0f, -3.0f,  3.0f,
			  0.0f,  0.0f, -2.0f,
			  2.0f,  0.0f,  0.0f,
			  BottomFacelets);

	glEnd();

	//pGLState->m_RotAngle += 0.5f;
}

static void HandleHeldKeys(GLFWwindow* Window)
{
	SGLState* pGLState = static_cast<SGLState*>(glfwGetWindowUserPointer(Window));

	//if (glfwGetKey(Window, GLFW_KEY_LEFT) == GLFW_PRESS)
	//{
	//	pGLState->m_RotAngle += 0.8f;
	//}
}

static void KeyCallback(GLFWwindow* Window, int Key, int ScanCode, int Action, int Mods)
{
	if (Action == GLFW_PRESS)
	{
		SGLState* pGLState = static_cast<SGLState*>(glfwGetWindowUserPointer(Window));

		switch (Key)
		{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(Window, GLFW_TRUE);
			break;
		}
	}
}

int main()
{
	// Initialize the library
	if (!glfwInit())
		return -1;

	// Create a windowed mode window and its OpenGL context
	GLFWwindow* Window = glfwCreateWindow(640, 480, "RubikView", NULL, NULL);
	if (!Window)
	{
		glfwTerminate();
		return -1;
	}

	SGLState GLState;
	glfwSetWindowUserPointer(Window, &GLState);

	glfwSetKeyCallback(Window, KeyCallback);

	// Make the window's context current
	glfwMakeContextCurrent(Window);

	glfwSwapInterval(1);
	InitGL();

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(Window))
	{
		int Width, Height;
		glfwGetFramebufferSize(Window, &Width, &Height);
		ReSizeGLScene(Width, Height);

		// Render here
		DrawGLScene(Window);

		// Swap front and back buffers
		glfwSwapBuffers(Window);

		// Poll for and process events
		glfwPollEvents();

		HandleHeldKeys(Window);

	}

	glfwTerminate();
	return 0;
}
