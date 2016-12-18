#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

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

void DrawFace(GLfloat cx, GLfloat cy, GLfloat cz, // color
	      GLfloat px, GLfloat py, GLfloat pz, // first corner
	      GLfloat ux, GLfloat uy, GLfloat uz, // first  vector for 1 facelet
	      GLfloat vx, GLfloat vy, GLfloat vz) // secont vector for 1 facelet
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
	glColor3f(cx, cy, cz);
	for (int i = 0; i < 3; ++i)
	{
		// Offset is used to skip the black outline only once.
		GLfloat ss = i + Offset;
		GLfloat sx = px + ss * vx;
		GLfloat sy = py + ss * vy;
		GLfloat sz = pz + ss * vz;

		for (int j = 0; j < 3; ++j)
		{
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
void DrawGLScene()
{
	static GLfloat s_RotAngle = 0.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();					// Reset The Current Modelview Matrix

	glTranslatef(0.0f, 0.0f, -21.0f);			// Translate Into The Screen 7.0 Units
	glRotatef(s_RotAngle, 0.0f, 1.0f, 0.0f);		// Rotate The cube around the Y axis
	glRotatef(s_RotAngle, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);

		// Top: white
		DrawFace( 1.0f,  1.0f,  1.0f,
			 -3.0f,  3.0f, -3.0f,
			  0.0f,  0.0f,  2.0f,
			  2.0f,  0.0f,  0.0f);

		// Front: red
		DrawFace( 1.0f,  0.0f,  0.0f,
			 -3.0f,  3.0f,  3.0f,
			  0.0f, -2.0f,  0.0f,
			  2.0f,  0.0f,  0.0f);

		// Right: blue
		DrawFace( 0.0f,  0.0f,  1.0f,
			  3.0f,  3.0f,  3.0f,
			  0.0f, -2.0f,  0.0f,
			  0.0f,  0.0f, -2.0f);

		// Back: orange
		DrawFace( 1.0f,  0.5f,  0.0f,
			  3.0f,  3.0f, -3.0f,
			  0.0f, -2.0f,  0.0f,
			 -2.0f,  0.0f,  0.0f);

		// Left: green
		DrawFace( 0.0f,  1.0f,  0.0f,
			 -3.0f,  3.0f, -3.0f,
			  0.0f, -2.0f,  0.0f,
			  0.0f,  0.0f,  2.0f);

		// Bottom: yellow
		DrawFace( 1.0f,  1.0f,  0.0f,
			 -3.0f, -3.0f,  3.0f,
			  0.0f,  0.0f, -2.0f,
			  2.0f,  0.0f,  0.0f);

	glEnd();

	s_RotAngle += 0.5f;
}

int main()
{
	// Initialize the library
	if (!glfwInit())
		return -1;

	// Create a windowed mode window and its OpenGL context
	GLFWwindow* Window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!Window)
	{
		glfwTerminate();
		return -1;
	}

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
		DrawGLScene();

		// Swap front and back buffers
		glfwSwapBuffers(Window);

		// Poll for and process events
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
