#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

#include <vector>

// Class that stores the density readings
// and other related info
class DensityMap {
private:
	// This should never change after initialization
	long long int dim;

	// IDs of buffers on the graphics card
	unsigned int cellVAO;
	unsigned int cellDensityTBO;
	unsigned int cellDensityBufferTexture;

	unsigned int lineVAO;
	unsigned int lineVBO;

	// Minimum value needed to draw a cell
	unsigned char threshold;

	// Creating the shaders for the cells in the cube
	// and for the lines of the border of the cube
	Shader cellShader;
	Shader lineShader;
public:
	// Constructor
	DensityMap(long long int dim);

	// Adds a line of data between p1 and p2
	void addLine(glm::vec3 p1, glm::vec3 p2, std::vector<unsigned char> vals);

	// Overwrites everything with value
	void clear(unsigned char value = 0);

	// Returns dim
	int getDim();

	// Draws to the screen and optionally clears the screen
	void draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model);

	// Set and get the threshold for drawing a cell
	void setThreshold(unsigned char value);
	unsigned char getThreshold();
};