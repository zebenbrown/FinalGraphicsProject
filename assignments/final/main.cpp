#include <cstdio>
#include <cmath>

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



vec2 mousePos = vec2(0);

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
        }
	}
}

void mouse_position_callback(GLFWwindow *window, double xpos, double ypos) {
	camera.handleMouse(xpos, ypos);
    mousePos.x = xpos;
    mousePos.y = ypos;
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
	//camera.handleScroll(yoffset);
}

int main() {
	auto window = Window("GALAXY");
	glfwSetInputMode(window.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window.window, key_callback);
	glfwSetCursorPosCallback(window.window, mouse_position_callback);
	glfwSetScrollCallback(window.window, scroll_callback);
    Line::loadShader();
	camera = Camera(vec3(), vec3(), 60.0f, vec2(Window::SCREEN_WIDTH, Window::SCREEN_HEIGHT));
    //disables axis gizmo
    camera.ui = false;

    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );


    //the 'default' position/rotation of the camera when no world is selected
    pair defaultCameraPosRot = Camera::loadString("[-19.075241, 9.822771, 25.880850, 0.000000, -23.500034, -416.899048]");
    camera.setPosRot(defaultCameraPosRot);
    //the camera angle when a world IS selected
    vec3 worldLookingAtRotation = vec3(0.000000f, -12.499994f, -377.298370f);


    int selectedWorld = 0;
    int lastWorld = selectedWorld;
    //when this is NOT -1.0f, a camera transition is happening
    float cameraTransitionStartTime = -1.0f;

    //sets the selectedWorld and updates other things to handle camera transitions
    auto setWorld = [&lastWorld, &selectedWorld, &window, &cameraTransitionStartTime](int world) {
        lastWorld = selectedWorld;
        selectedWorld = world;
        cameraTransitionStartTime = window.getTime();
    };

    //set this to false if you want regular camera WASD flight
    camera.disable = true;


    //shader loading
    Shader torusShader = Shader("assets/torus");

    Shader textRenderingShader = Shader("assets/textRendering");
    textRendering textRendering;
    textRendering.loadText("assets/super-mario-256.ttf");

    Shader hudElementShader = Shader("assets/hudElement");
    hudElementShader.use();

	Shader sphereShader = Shader("assets/sphere");
	sphereShader.use();
	sphereShader.setInt("sphereMapTex", 0);

    Texture2d wiiPointer = Texture2d("assets/wii-pointer.png");
	Texture2d skyTexture = Texture2d("assets/skysphere.png");
    Texture2d asteroidTexture = Texture2d("assets/asteroid.png");

    struct World {
        Texture2d texture;
        string name;
        float speed = 1.0f;
    };

    World worlds[5];
    //loads all 5 world textures
    for(int i = 1; i <= 5; i++) worlds[i - 1].texture = Texture2d("assets/planets/Planet"+to_string(i)+".png");
    //set some misc data about each world
    worlds[0].name = "Miralton";
    worlds[0].speed = 1.3445f;
    worlds[1].name = "Drek";
    worlds[1].speed = 2.3482f;
    worlds[2].name = "Wave World";
    worlds[2].speed = 0.6485f;
    worlds[3].name = "Pumplanet";
    worlds[3].speed = 0.9348f;
    worlds[4].name = "Iridon";
    worlds[4].speed = 1.8494f;

    //generate sphere for worlds and sky
	MeshData sphereMeshData;
	createSphere(1, 1024, &sphereMeshData);
	auto sphereMesh = ProceduralMesh(sphereMeshData);

    struct Asteroid {
        float angle = 0.0f; //degrees
        float dist = 150.0f;
        float ringShift = 0.0f; //how many degrees off the normal asteroid ring it is
        MeshData meshData;
        ProceduralMesh mesh;
    };

    const int ASTEROID_COUNT = 1000;
    Asteroid asteroids[ASTEROID_COUNT];
    for(auto & asteroid : asteroids) {
        createAsteroid(RandomRange(0.01f, 0.2f), 0.25f, (int) (RandomRange(10, 16)), &asteroid.meshData);
        asteroid.mesh = ProceduralMesh(asteroid.meshData);
        asteroid.angle = RandomRange(0, 360.0f);
        asteroid.ringShift = RandomRange(-5.0f, 5.0f);
        asteroid.dist = RandomRange(12, 25);
    }

    //generate torus for rings
    TorusGen torus = TorusGen(0.01f, 1.0f, 200, 200);

    //orthographic projection for HUD elements (text, wii pointer)
    mat4 hudProjection = ortho(0.0f, static_cast<float>(Window::SCREEN_WIDTH), 0.0f, static_cast<float>(Window::SCREEN_HEIGHT), -1.0f, 1.0f);

	//Render loop
	while (!glfwWindowShouldClose(window.window)) {
		glfwPollEvents();
		float deltaTime = window.update();
		float time = window.getTime();
		camera.update(window.window, deltaTime);
		mat4 viewProj = camera.proj * camera.view;


        //skybox/spheremap
        sphereShader.use();
        glActiveTexture(GL_TEXTURE0);
        skyTexture.bind();
        sphereShader.setMat4("viewProj", viewProj);
        sphereShader.setMat4("model", Object::scale(100, 100, 100));
        sphereMesh.draw();


        //rings setup
        float ringAngle = 30.0f;
        mat4 torusModel = Object::scale(20, 20, 20);
        torusShader.use();
        torusModel = glm::rotate(torusModel, radians(90.0f + ringAngle), vec3(1, 0, 0));

        torusShader.setMat4("modelMatrix", torusModel);
        torusShader.setMat4("view", viewProj);

        //draw rings
        for (int i = 1; i <= 5; i++)
        {
            torusShader.setVec3("offset", vec3(i - (i * 0.8f)));
            torus.draw();
        }



        //0 = no world is being hovered, otherwise its set to the world that IS being hovered (used for world name text)
        int hoveredWorld = 0;
        vec3 worldPositions[5];
        sphereShader.use();
        for(int i = 1; i <= 5; i++) {
            //calculate a point along the rings
            float dist = i * 0.2f * 20;
            //how fast they go around the rings
            float angle = (time + 88.0f) * worlds[i - 1].speed;
            //calculate point on flat rings
            vec3 pos = vec3(dist * cos(radians(angle)), 0.0f, dist * sin(radians(angle)));
            //rotate it up
            mat4 rotMat = glm::rotate(mat4(1), radians(ringAngle), vec3(1, 0, 0));
            pos = rotMat * vec4(pos, 1.0f);
            //save world positions for later
            worldPositions[i - 1] = pos;
            mat4 m = Object::translate(pos.x, pos.y, pos.z);


            //
            //check if you're hovering over the spheres
            //
            //hover dist is how many pixels away will it START scaling
            //maxScaleDist is how many pixels away it will STOP scaling and be at hoverScale (hover size)
            //scale is default size
            float hoverDist = 40, maxScaleDist = 25, scale = 1.5f, hoverScale = 1.75f;
            if(selectedWorld == 0 && cameraTransitionStartTime == -1.0f) { //make sure camera is in default position AND its not currently transitioning

                //ChatGPT helped with the concept of this section but I (Drew) wrote the code myself

                //get sphere screen coordinates

                mat4 mvp = viewProj * m;
                //transform the sphere's center to clip space
                //Note: I do not know why I needed the '0.1f / i' but it makes it work soooo
                vec4 clipSpace = mvp * vec4(pos * (0.1f / static_cast<float>(i)), 1.0f);
                //perspective division to get normal device coordinates (-1 to 1 range)
                vec3 ndc = vec3(clipSpace.x / clipSpace.w, clipSpace.y / clipSpace.w, clipSpace.z / clipSpace.w);
                //convert normal device coordinates to screen coordinates
                vec2 screenCoords = vec2((1.0f + ndc.x) * 0.5f * Window::SCREEN_WIDTH, (1.0f - ndc.y) * 0.5f * Window::SCREEN_HEIGHT);
                //distance from mouse to sphere
                float d = glm::length(mousePos - screenCoords);
                if(d <= hoverDist) {
                    //set the hovered world for the world worldName text
                    hoveredWorld = i;

                    //Select a world
                    if(glfwGetMouseButton(window.window, GLFW_MOUSE_BUTTON_LEFT)) {
                        setWorld(i);
                    }
                    if(d > maxScaleDist && d <= hoverDist) {
                        //linear interp between hoverDist and maxScaleDist
                        float t = (hoverDist - d) / (hoverDist - maxScaleDist);
                        float s = scale + t * (hoverScale - scale);
                        //apply partial scaling (based on mouse distance from world)
                        m*=Object::scale(s, s, s);
                    } else { //apply full scaling
                        m*=Object::scale(hoverScale, hoverScale, hoverScale);
                    }
                } else { //apply default scaling
                    m*=Object::scale(scale, scale, scale);
                }
            } else { //apply default scaling
                m*=Object::scale(scale, scale, scale);
            }

            //draw "worlds"
            worlds[i-1].texture.bind();
            sphereShader.setMat4("model", m);
            sphereMesh.draw();
        }

        //draw asteroid ring
        asteroidTexture.bind();
        float asteroidRingAngle = 20.0f;
        for(auto & asteroid : asteroids) {
            vec3 pos = vec3(cos(radians(asteroid.angle + time)), 0, sin(radians(asteroid.angle + time))) * asteroid.dist;
            sphereShader.setMat4("model", Object::rotate(0, radians(-asteroidRingAngle + asteroid.ringShift), 0) * Object::translate(pos.x, pos.y, pos.z));
            asteroid.mesh.draw();
        }

        //Camera handler (when a world is selected)
        if(camera.disable && selectedWorld != 0) { //default camera angle
            if(cameraTransitionStartTime == -1.0f) { //if camera transition is NOT active
                camera.pos = worldPositions[selectedWorld - 1];
                camera.pos.x-=5;
                camera.pos.y+=1.5f;
                camera.pos.z+=2;
                camera.rot = worldLookingAtRotation;
            }
            //Unselect a world
            if(cameraTransitionStartTime == -1.0f && glfwGetMouseButton(window.window, GLFW_MOUSE_BUTTON_RIGHT)) {
                setWorld(0);
            }
        }

        //function to set the start position/rotation and the end position/rotation of the camera transition
        auto setPosRot = [&worldLookingAtRotation, &worldPositions, &defaultCameraPosRot](int world, vec3 &pos, vec3 &rot) {
            if(world == 0) {
                pos = defaultCameraPosRot.first;
                rot = defaultCameraPosRot.second;
            } else {
                pos = worldPositions[world - 1];
                pos.x-=5;
                pos.y+=1.5f;
                pos.z+=2;
                rot = worldLookingAtRotation;
            }
        };

        //Camera transition handler
        if(cameraTransitionStartTime != -1.0f) { //check if there is an active camera transition
            float duration = 1.0f;
            vec3 startPos, startRot, endPos, endRot;

            //set the position/rotation of the start/end points of the camera transition
            setPosRot(lastWorld, startPos, startRot);
            setPosRot(selectedWorld, endPos, endRot);

            //basic lerp function but put inside the camera
            camera.lerpCamera(cameraTransitionStartTime, time, duration, pair(startPos, startRot), pair(endPos, endRot));
            if(time - cameraTransitionStartTime > duration) {
                cameraTransitionStartTime = -1.0f; //set camera transition to be over/unstarted before the next one
            }
        }


        //HUD START
        glDisable(GL_DEPTH_TEST);

        //Screen Text
        textRenderingShader.use();
        textRenderingShader.setMat4("projection", hudProjection);

        string worldName;
        if(hoveredWorld == 0) {
            worldName = selectedWorld == 0 ? "Mario Galaxy Map" : worlds[selectedWorld - 1].name;
        } else {
            worldName = worlds[hoveredWorld - 1].name;
        }

        //drop shadow for the text
        vec3 textColor = vec3(1, 1, 1);
        textRenderingShader.setVec3("textColor", textColor);
        textRendering.RenderText(textRenderingShader, worldName, 28.0f, 28.0f, 1.0f, textColor);
        //the actual text
        textColor = vec3(0.2f, 0.5f, 1);
        textRenderingShader.setVec3("textColor", textColor);
        textRendering.RenderText(textRenderingShader, worldName, 30.0f, 30.0f, 1.0f, textColor);

        //Wii pointer
        if(!camera.lock && camera.disable) {
            glBindVertexArray(*Texture2d::getVAO());
            hudElementShader.use();
            hudElementShader.setMat4("projection", hudProjection);
            float pointerScale = 0.1f;
            hudElementShader.setMat4("model", Object::translate(mousePos.x + wiiPointer.getWidth() * pointerScale * 0.5f, Window::SCREEN_HEIGHT - mousePos.y - wiiPointer.getHeight() * pointerScale, 0)
                    * Object::scale(wiiPointer.getWidth() * pointerScale, wiiPointer.getHeight() * pointerScale, 1));
            wiiPointer.bind();
            wiiPointer.draw();
        }

        drawAxisGizmo();

        glEnable(GL_DEPTH_TEST);
        //HUD END


		glfwSwapBuffers(window.window);
	}
	printf("Shutting down...");
}
