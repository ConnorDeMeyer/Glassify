
#include <cstdio>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ImGui_GlasDemo.h"

void error_callback(int, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void FramebufferSizeCallback(GLFWwindow*, int width, int height)
{
	if (width == 0 || height == 0)
		return;

	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main()
{
	glfwInit();
	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	auto window = glfwCreateWindow(1280, 720, "Window", nullptr, nullptr);

	if (!window)
		return -1;

	glfwMaximizeWindow(window);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
	glfwSetKeyCallback(window, key_callback);
	gladLoadGL();
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		return -1;

	InitializeImgui(window);

	while (!glfwWindowShouldClose(window))
	{
		int width{};
		int height{};
		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		DrawImgui();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	DestroyImgui();

	glfwDestroyWindow(window);

	glfwTerminate();
}