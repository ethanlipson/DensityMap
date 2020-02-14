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
	int dim;

	// IDs of buffers on the graphics card
	unsigned int cellVAO;
	unsigned int cellPositionVBO, cellDensityVBO;

	unsigned int lineVAO;
	unsigned int lineVBO;

	// Minimum value needed to draw a cell
	unsigned char threshold;

	// Number of vertices in the graphics card
	// This is needed because not all vertices
	// are necessarily sent to the graphics card
	int numVertices;

	// Creating the shaders for the cells in the cube
	// and for the lines of the border of the cube
	Shader cellShader;
	Shader lineShader;
public:
	// 3D array that stores the data
	std::vector<std::vector<std::vector<unsigned char>>> cells;

	// Constructor
	DensityMap(int dim);

	// Adds a line of data between p1 and p2
	void addLine(glm::vec3 p1, glm::vec3 p2, std::vector<unsigned char> vals);

	// Overwrites everything with value
	void clear(unsigned char value = 0);

	// Returns dim
	int getDim();

	// Draws to the screen and optionally clears the screen
	void draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model);

	// Used by DensityMap::draw()
	std::vector<float> getVertexPositions();
	std::vector<unsigned char> getVertexDensities();

	// Set and get the threshold for drawing a cell
	void setThreshold(unsigned char value);
	unsigned char getThreshold();

	// Updates the vertices on the graphics card
	// -----
	// This function is pretty slow right now (around 100 milliseconds)
	// because it writes several megabytes of data at once to the graphics card,
	// so don't call it too frequently
	void updateVertexBuffers();
};