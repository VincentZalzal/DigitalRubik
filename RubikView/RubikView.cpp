#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

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

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(Window))
	{
		// Render here
		glClear(GL_COLOR_BUFFER_BIT);

		// Swap front and back buffers
		glfwSwapBuffers(Window);

		// Poll for and process events
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
