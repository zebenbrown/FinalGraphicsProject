#include <stdio.h>
#include <math.h>

#include "cobb/window.hpp"

#include <ew/external/glad.h>
#include <ew/ewMath/ewMath.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "cobb/camera.hpp"
#include "cobb/geometry.hpp"
#include "cobb/line.hpp"
#include "cobb/mesh.hpp"
#include "cobb/texture2d.hpp"
#include "cobb/TorusGen.h"
#include "cobb/textRendering.h"

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

		vec3 p = camera.forward * vec3(compassScale) + camera.pos;

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

float startTime = -1.0f;
auto startPosRot = Camera::loadString("[0, 0, 0, 0, 0, 0]");
auto endPosRot = Camera::loadString("[0, 0, 0, 0, 0, 0]");

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	camera.handleKeyboard(key, action);
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_ESCAPE) {
			if (camera.lock) {
				glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			} else {
				glClearColor(0.0f, 0.1f, 0.2f, 1.0f);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
		} else if(key == GLFW_KEY_P) {
            cout << camera.getString() << endl;
        } else if(key == GLFW_KEY_L) {
            startTime = static_cast<float>(glfwGetTime());
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

    Shader torusShader = Shader("assets/torus");
    Shader textRenderingShader = Shader("assets/textRendering");
    TorusGen torus(0.01f, 1.0f, 200, 200);

    textRendering textRendering;

    textRendering.loadText("assets/super-mario-256.ttf");
	Shader skyShader = Shader("assets/sphere");
	skyShader.use();
	skyShader.setInt("sphereMapTex", 0);
	Texture2d skyTexture = Texture2d("assets/skysphere.png");

	MeshData sphereMeshData;
	createSphere(1.0f, 1024, &sphereMeshData);
	auto sphereMesh = ProceduralMesh(sphereMeshData);

	mat4 model = Object::scale(100, 100, 100);

    //set start/end position/rotation here
    startPosRot = Camera::loadString("[0.025197, 12.000000, 34.181889, 0.000000, -19.500004, -440.999451]");
    endPosRot = Camera::loadString("[-24.538515, 29.393368, -15.470002, 0.000000, -43.699879, -340.496185]");

	//Render loop
	while (!glfwWindowShouldClose(window.window)) {
		glfwPollEvents();
		float deltaTime = window.update();
		float time = window.getTime();
		camera.update(window.window, deltaTime);
		mat4 viewProj = camera.proj * camera.view;

        mat4 torusModel = model;
        torusShader.use();

        torusModel = glm::rotate(torusModel, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        torusModel = glm::scale(torusModel, glm::vec3(0.2f));

        torusShader.setMat4("modelMatrix", torusModel);
        torusShader.setMat4("view", viewProj);

        for (int i = 5; i > 0; --i)
        {
            torusShader.setVec3("offset", glm::vec3(i - (i * 0.8)));
            torus.draw();
        }

		skyShader.use();
		glActiveTexture(GL_TEXTURE0);
		skyTexture.bind();
		skyShader.setMat4("viewProj", viewProj);
		skyShader.setMat4("model", model);
		sphereMesh.draw();


        //Camera movement (press 'P' to record a position/rotation)
        //Copy paste the string into the one using the "Camera::loadString()" function to set a position/rotation
        //Press 'L' to activate the camera movement
        // duration is how fast the camera will move
        if(startTime != -1.0f) {
            float duration = 1.0f;
            camera.lerpCamera(startTime, time, duration, startPosRot, endPosRot);
            if(time - startTime > duration) {
                startTime = -1.0f;
            }
        }


		drawAxisGizmo();


        std::string sampleText = "Mario Galaxy Map";
        mat4 textProjection = glm::ortho(0.0f, static_cast<float>(Window::SCREEN_WIDTH), 0.0f, static_cast<float>(Window::SCREEN_HEIGHT));
        textRenderingShader.use();
        textRenderingShader.setMat4("projection", textProjection);
        
        vec3 textColor = vec3(1, 0, 0);
        textRenderingShader.setVec3("textColor", textColor);
        textRendering.RenderText(textRenderingShader, sampleText, 5.0f, 1030.0f, 1.0f, textColor);

		glfwSwapBuffers(window.window);
	}
	printf("Shutting down...");
}
