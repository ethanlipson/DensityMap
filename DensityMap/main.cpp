#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "shader.h"
#include "camera.h"

#include "densitymap.h"

#define PI 3.141592653589

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

// Keyboard and mouse input functions
void cursorPosMovementCallback(GLFWwindow* window, double xpos, double ypos);
void cursorPosRotationCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void processKeyboardInput(GLFWwindow* window);

// Demo functions to show what the volume map looks like
void sphereDemo(DensityMap& grid);
void fanDemo(DensityMap& grid);

// Used in the mouse movement callbacks
double lastMouseX;
double lastMouseY;
bool firstMouse;

float rotationX;
float rotationY;

bool mousePressed;

// Creating a Camera object
Camera cam;

const bool ROTATE_GRID = true;

int main() {
	// Window title
	std::string windowTitle = "Density Map";

	// Variables for measuring FPS
	int numFrames = 0;
	double lastFPSUpdate = 0;

	// Initializing the OpenGL context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	// Needed to fix compilation on macOS
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Creating the window object
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, windowTitle.c_str(), NULL, NULL);
	
	// If the window is not created (for any reason)
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Setting callbacks
	glfwMakeContextCurrent(window);

	if (ROTATE_GRID) {
		glfwSetCursorPosCallback(window, cursorPosRotationCallback);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
	}
	else {
		glfwSetCursorPosCallback(window, cursorPosMovementCallback);
	}

	if (!ROTATE_GRID) {
		// Lock the cursor to the center of the window
		// and make it invisible
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	
	// Load the OpenGL functions from the graphics card
	if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Initializing mouse info
	lastMouseX = SCR_WIDTH / 2.0;
	lastMouseY = SCR_HEIGHT / 2.0;
	firstMouse = true;

	// Creating the density map
	int dim = 100;
	DensityMap grid(dim);

	// Add a sphere to the center of the grid
	fanDemo(grid);

	// Add all non-empty cells to the map
	grid.setThreshold(1);

	// Main event loop
	while (!glfwWindowShouldClose(window)) {
		double currentFrame = glfwGetTime();
		cam.deltaTime = currentFrame - cam.lastFrame;
		cam.lastFrame = currentFrame;

		// Self-explanatory
		processKeyboardInput(window);

		// Clears the screen and fills it a dark grey color
		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// Creating matrices to transform the vertices into NDC (screen) coordinates
		// between -1 and 1 that OpenGL can use
		glm::mat4 projection = glm::perspective<float>(glm::radians(cam.fov), float(SCR_WIDTH) / SCR_HEIGHT, 0.01, 500.0);
		glm::mat4 view = cam.getViewMatrix();
		glm::mat4 model = glm::mat4(1.0);

		if (ROTATE_GRID) {
			model = glm::rotate(model, rotationX, glm::vec3(1, 0, 0));
			model = glm::rotate(model, rotationY, glm::vec3(0, 1, 0));
		}

		// Draw the density map and the surrounding cube
		grid.draw(projection, view, model);

		// Used to make camera move speed consistent
		cam.prevPos = cam.position;

		// Update the screen
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Measuring FPS
		if (glfwGetTime() - lastFPSUpdate >= 1) {
			std::string newTitle = windowTitle + " (" + std::to_string(numFrames) + " FPS)";
			glfwSetWindowTitle(window, newTitle.c_str());

			lastFPSUpdate = glfwGetTime();
			numFrames = 0;
		}

		// Increment the number of frames in the past second
		numFrames++;
	}

	// GLFW cleanup
	glfwTerminate();
}

void processKeyboardInput(GLFWwindow *window) {
	// If shift is held down, then the camera moves faster
	bool sprinting = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);

	// WASD + Q and E movement
	if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, true);

	if (ROTATE_GRID) {
		if (glfwGetKey(window, GLFW_KEY_R)) {
			rotationX = 0;
			rotationY = 0;
		}
	}
	else {
		if (glfwGetKey(window, GLFW_KEY_W))
			cam.processKeyboard(FORWARD, sprinting);
		if (glfwGetKey(window, GLFW_KEY_S))
			cam.processKeyboard(BACKWARD, sprinting);
		if (glfwGetKey(window, GLFW_KEY_A))
			cam.processKeyboard(LEFT, sprinting);
		if (glfwGetKey(window, GLFW_KEY_D))
			cam.processKeyboard(RIGHT, sprinting);
		if (glfwGetKey(window, GLFW_KEY_Q))
			cam.processKeyboard(DOWN, sprinting);
		if (glfwGetKey(window, GLFW_KEY_E))
			cam.processKeyboard(UP, sprinting);

		// Hold C to zoom in
		if (glfwGetKey(window, GLFW_KEY_C)) {
			cam.fov = 30;
		}
		else {
			cam.fov = 70;
		}
	}
}

void cursorPosMovementCallback(GLFWwindow* window, double xpos, double ypos) {
	// Ensures that the camera faces forward on startup
	if (firstMouse) {
		lastMouseX = xpos;
		lastMouseY = ypos;
		firstMouse = false;
	}

	// Updating the camera angle
	double xoffset = xpos - lastMouseX;
	double yoffset = lastMouseY - ypos;
	lastMouseX = xpos;
	lastMouseY = ypos;

	cam.processMouseMovement(xoffset, yoffset);
}

void cursorPosRotationCallback(GLFWwindow* window, double xpos, double ypos) {
	// Ensures that the cube faces forward on startup
	if (firstMouse) {
		lastMouseX = xpos;
		lastMouseY = ypos;
		firstMouse = false;
	}

	// Updating the camera angle
	double xoffset = xpos - lastMouseX;
	double yoffset = lastMouseY - ypos;
	lastMouseX = xpos;
	lastMouseY = ypos;

	if (mousePressed) {
		rotationY += xoffset / 200.0;
		rotationX -= yoffset / 200.0;

		// 1.5 is a bit less than pi / 2
		if (rotationX > 1.5) {
			rotationX = 1.5;
		}

		if (rotationX < -1.5) {
			rotationX = -1.5;
		}
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		mousePressed = true;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		mousePressed = false;
	}
}

void sphereDemo(DensityMap& grid) {
	// Adds a sphere to the center of the volume map

	int dim = grid.getDim();

	float radius = 0.3;

	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			for (int k = 0; k < dim; k++) {
				float xd = i - ((dim - 1) / 2.0);
				float yd = j - ((dim - 1) / 2.0);
				float zd = k - ((dim - 1) / 2.0);

				float mxd = (dim - 1) / 2.0;
				float myd = (dim - 1) / 2.0;
				float mzd = (dim - 1) / 2.0;

				float distance = sqrt(xd * xd + yd * yd + zd * zd);
				float maxDistance = sqrt(mxd * mxd + myd * myd + mzd * mzd);
				float shade = (maxDistance - distance) / maxDistance;
				shade = shade * shade;

				if (distance < radius * dim) {
					grid.writeCell(i, j, k, static_cast<unsigned char>(shade * 255));
				}
			}
		}
	}
}

void fanDemo(DensityMap& grid) {
	// Adds a fan shape to the volume map
	// using the DensityMap::addLine() function

	glm::vec3 vertex = { 0.5, 0.5, 0.5 };

	float a1 = 1;
	float a2 = 1;

	float r = 0.3;

	for (; a2 <= 3; a2 += 0.01) {
		float x = r * sin(a1) * cos(a2);
		float y = r * sin(a1) * sin(a2);
		float z = r * cos(a1);

		std::vector<unsigned char> vals;

		for (int i = 0; i < 1000; i++) {
			vals.push_back(255);
		}

		grid.writeLine(vertex, vertex + glm::vec3(x, y, z), vals);
	}
}