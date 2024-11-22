#include <stdio.h>
#include <math.h>

#include "cobb/window.hpp"

#include <ew/external/glad.h>
#include <ew/ewMath/ewMath.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "cobb/camera.hpp"
#include "cobb/line.hpp"

const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;

using namespace cobb;
using namespace std;

Camera camera;

void drawAxisGizmo() {
	if(camera.ui && lineShader != nullptr) {
		glDisable(GL_DEPTH_TEST);

		//higher number = less stuttering, can't be above ~950 because depth will start cutting it off, 900 to be safe
		const float compassScale = 900.0f;

		vec3 p = camera.forward * vec3(compassScale) + camera._position;

		Line xAxis = Line(p, p + vec3(0.05f * compassScale, 0, 0), vec4(1, 0, 0, 1));
		Line yAxis = Line(p, p + vec3(0, 0.05f * compassScale, 0), vec4(0, 1, 0, 1)); //up
		Line zAxis = Line(p, p + vec3(0, 0, 0.05f * compassScale), vec4(0, 0, 1, 1));

		lineShader->use();
		lineShader->setMat4("viewProj", camera.proj * camera.view);
		glBindVertexArray(*Line::getVAO());

		xAxis.draw(&camera);
		yAxis.draw(&camera);
		zAxis.draw(&camera);

		glEnable(GL_DEPTH_TEST);
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	camera.handleKeyboard(key, action);
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE)
		{
			if (camera.lock)
			{
				glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			} else
			{
				glClearColor(0.0f, 0.1f, 0.2f, 1.0f);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
		}
	}
}

void mouse_position_callback(GLFWwindow *window, double xpos, double ypos) {
	camera.handleMouse(xpos, ypos);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
	camera.handleScroll(yoffset);
}

int main() {
	auto window = Window("GALAXY");
	glfwSetInputMode(window.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window.window, key_callback);
	glfwSetCursorPosCallback(window.window, mouse_position_callback);
	glfwSetScrollCallback(window.window, scroll_callback);
	camera = Camera(vec3(0.5f, 12.0f, 31.3f), vec3(0, 0, -450.0f), 60.0f, vec2(Window::SCREEN_WIDTH, Window::SCREEN_HEIGHT));
	Line::loadShader();


	
	//Render loop
	while (!glfwWindowShouldClose(window.window)) {
		glfwPollEvents();
		float deltaTime = window.update();
		float time = window.getTime();
		camera.update(window.window, deltaTime);
		mat4 viewProj = camera.proj * camera.view;

		drawAxisGizmo();

		glfwSwapBuffers(window.window);
	}
	printf("Shutting down...");
}
