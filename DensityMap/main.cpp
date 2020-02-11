#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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
void processKeyboardInput(GLFWwindow* window);

// Updates the vertices on the graphics card
// -----
// This function is pretty slow right now (a few hundred milliseconds)
// because it writes several megabytes of data at once to the graphics card,
// but it will be optimized soon
void updateVertexBuffer(unsigned int& VBO, DensityMap& grid);

// Demo functions to show what the volume map looks like
void sphereDemo(DensityMap& grid);
void fanDemo(DensityMap& grid);

// Used in the mouse movement callback
double lastMouseX;
double lastMouseY;
bool firstMouse;

// Creating a Camera object
Camera cam;

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
	glfwSetCursorPosCallback(window, cursorPosMovementCallback);

	// Lock the cursor to the center of the window
	// and make it invisible
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	// Load the OpenGL functions from the graphics card
	if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Creating the shaders for the cells in the cube
	// and for the lines of the border of the cube
	Shader cellShader("cells.vs", "cells.fs");
	Shader lineShader("lines.vs", "lines.fs");

	// Allows blending (translucent drawing)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initializing mouse info
	lastMouseX = SCR_WIDTH / 2.0;
	lastMouseY = SCR_HEIGHT / 2.0;
	firstMouse = true;

	// Creating the density map
	int dim = 21;
	DensityMap grid(dim);
	
	// (Optional) Adds a sphere to the center of the density map
	sphereDemo(grid);
	
	// Get the vertices from the volume map
	// in a form useful to OpenGL
	std::vector<float> cellPositions = grid.getVertexPositions();
	std::vector<unsigned char> cellDensities = grid.getVertexDensities();
	
	// Initializing the buffers storing the vertices
	// of the volume map on the graphics card
	unsigned int cellVAO, cellPositionVBO, cellDensityVBO;
	glGenBuffers(1, &cellPositionVBO);
	glGenBuffers(1, &cellDensityVBO);
	glGenVertexArrays(1, &cellVAO);
	
	glBindVertexArray(cellVAO);
	
	// Cell positions
	
	glBindBuffer(GL_ARRAY_BUFFER, cellPositionVBO);
	glBufferData(GL_ARRAY_BUFFER, cellPositions.size() * sizeof(float), cellPositions.data(), GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	
	// Data in each cell
	
	glBindBuffer(GL_ARRAY_BUFFER, cellDensityVBO);
	glBufferData(GL_ARRAY_BUFFER, cellDensities.size() * sizeof(unsigned char), cellDensities.data(), GL_STATIC_DRAW);
	
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(unsigned char), 0);
	glEnableVertexAttribArray(1);

	// Array containing the coordinates of the vertices
	// of the white lines
	float lines[72] = {
		-5, -5, -5,  5, -5, -5,
		-5,  5, -5,  5,  5, -5,
		-5, -5,  5,  5, -5,  5,
		-5,  5,  5,  5,  5,  5,

		 // -----

		-5, -5, -5, -5,  5, -5,
		 5, -5, -5,  5,  5, -5,
		-5, -5,  5, -5,  5,  5,
		 5, -5,  5,  5,  5,  5,

		 // -----

		-5, -5, -5, -5, -5,  5,
		 5, -5, -5,  5, -5,  5,
		-5,  5, -5, -5,  5,  5,
		 5,  5, -5,  5,  5,  5,
	};

	// Initializing the buffer storing the vertices
	// of the white lines on the graphics card
	unsigned int lineVAO, lineVBO;
	glGenBuffers(1, &lineVBO);
	glGenVertexArrays(1, &lineVAO);

	glBindVertexArray(lineVAO);

	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

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
		glm::dmat4 projection = glm::perspective(glm::radians(cam.fov), double(SCR_WIDTH) / SCR_HEIGHT, 0.01, 500.0);
		glm::dmat4 camView = cam.getViewMatrix();
		
		int dim = grid.getDim();
		glm::dmat4 model = glm::scale(glm::dmat4(1.0), glm::dvec3(10.0 / (dim - 1), 10.0 / (dim - 1), 10.0 / (dim - 1)));
		model = glm::translate(model, glm::dvec3(-(dim - 1) / 2.0, -(dim - 1) / 2.0, -(dim - 1) / 2.0));
		
		// Drawing the volume map
		cellShader.use();
		cellShader.setMat4("projection", projection);
		cellShader.setMat4("view", camView);
		cellShader.setMat4("model", model);
		
		glBindVertexArray(cellVAO);
		glDrawArrays(GL_TRIANGLES, 0, cellDensities.size());
		
		// Drawing the white lines
		lineShader.use();
		lineShader.setMat4("projection", projection);
		lineShader.setMat4("view", camView);
		lineShader.setMat4("model", glm::dmat4(1.0));
		
		glBindVertexArray(lineVAO);
		glDrawArrays(GL_LINES, 0, 24);

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

void cursorPosMovementCallback(GLFWwindow* window, double xpos, double ypos) {
	// Ensures that the viewer faces forward on startup
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
					grid.cells[i][j][k] = static_cast<unsigned char>(shade * 255);
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

		grid.addLine(vertex, vertex + glm::vec3(x, y, z), vals);
	}
}

void updateVertexBuffer(unsigned int& VBO, DensityMap& grid) {
	// Gets the vertices from the density map
	std::vector<unsigned char> densities = grid.getVertexDensities();

	// Writes the vertices to the vertex buffer on the graphics card
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, densities.size() * sizeof(unsigned char), densities.data());
}
