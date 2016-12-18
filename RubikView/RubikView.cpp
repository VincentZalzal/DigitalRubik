#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

// Resize And Initialize The GL Window
GLvoid ReSizeGLScene(int width, int height)
{
	if (height == 0)					// Prevent A Divide By Zero By
	{
		height = 1;					// Making Height Equal One
	}

	glViewport(0, 0, width, height);			// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);				// Select The Projection Matrix
	glLoadIdentity();					// Reset The Projection Matrix

								// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat) width / (GLfloat) height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);				// Select The Modelview Matrix
	glLoadIdentity();					// Reset The Modelview Matrix
}

// All Setup For OpenGL Goes Here
void InitGL(GLvoid)
{
	glShadeModel(GL_SMOOTH);				// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);			// Black Background
	glClearDepth(1.0f);					// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);				// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);					// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
}

// Here's Where We Do All The Drawing
void DrawGLScene(GLvoid)
{
	static float s_RotAngle = 0.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();					// Reset The Current Modelview Matrix

	glTranslatef(0.0f, 0.0f, -7.0f);			// Translate Into The Screen 7.0 Units
	glRotatef(s_RotAngle, 0.0f, 1.0f, 0.0f);		// Rotate The cube around the Y axis
	glRotatef(s_RotAngle, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);					// Draw a cube with quads
		glColor3f(0.0f, 1.0f, 0.0f);			// Color Blue
		glVertex3f(1.0f, 1.0f, -1.0f);			// Top Right Of The Quad (Top)
		glVertex3f(-1.0f, 1.0f, -1.0f);			// Top Left Of The Quad (Top)
		glVertex3f(-1.0f, 1.0f, 1.0f);			// Bottom Left Of The Quad (Top)
		glVertex3f(1.0f, 1.0f, 1.0f);			// Bottom Right Of The Quad (Top)
		glColor3f(1.0f, 0.5f, 0.0f);			// Color Orange
		glVertex3f(1.0f, -1.0f, 1.0f);			// Top Right Of The Quad (Bottom)
		glVertex3f(-1.0f, -1.0f, 1.0f);			// Top Left Of The Quad (Bottom)
		glVertex3f(-1.0f, -1.0f, -1.0f);		// Bottom Left Of The Quad (Bottom)
		glVertex3f(1.0f, -1.0f, -1.0f);			// Bottom Right Of The Quad (Bottom)
		glColor3f(1.0f, 0.0f, 0.0f);			// Color Red
		glVertex3f(1.0f, 1.0f, 1.0f);			// Top Right Of The Quad (Front)
		glVertex3f(-1.0f, 1.0f, 1.0f);			// Top Left Of The Quad (Front)
		glVertex3f(-1.0f, -1.0f, 1.0f);			// Bottom Left Of The Quad (Front)
		glVertex3f(1.0f, -1.0f, 1.0f);			// Bottom Right Of The Quad (Front)
		glColor3f(1.0f, 1.0f, 0.0f);			// Color Yellow
		glVertex3f(1.0f, -1.0f, -1.0f);			// Top Right Of The Quad (Back)
		glVertex3f(-1.0f, -1.0f, -1.0f);		// Top Left Of The Quad (Back)
		glVertex3f(-1.0f, 1.0f, -1.0f);			// Bottom Left Of The Quad (Back)
		glVertex3f(1.0f, 1.0f, -1.0f);			// Bottom Right Of The Quad (Back)
		glColor3f(0.0f, 0.0f, 1.0f);			// Color Blue
		glVertex3f(-1.0f, 1.0f, 1.0f);			// Top Right Of The Quad (Left)
		glVertex3f(-1.0f, 1.0f, -1.0f);			// Top Left Of The Quad (Left)
		glVertex3f(-1.0f, -1.0f, -1.0f);		// Bottom Left Of The Quad (Left)
		glVertex3f(-1.0f, -1.0f, 1.0f);			// Bottom Right Of The Quad (Left)
		glColor3f(1.0f, 0.0f, 1.0f);			// Color Violet
		glVertex3f(1.0f, 1.0f, -1.0f);			// Top Right Of The Quad (Right)
		glVertex3f(1.0f, 1.0f, 1.0f);			// Top Left Of The Quad (Right)
		glVertex3f(1.0f, -1.0f, 1.0f);			// Bottom Left Of The Quad (Right)
		glVertex3f(1.0f, -1.0f, -1.0f);			// Bottom Right Of The Quad (Right)
	glEnd();						// End drawing cube with quads

	s_RotAngle += 0.9f;
}

int main(void)
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
