#pragma once

#include <vector>

#include <glm/glm.hpp>

// Class that stores the density readings
// and other related info
class DensityMap {
private:
	// This should never change after initialization
	int dim;

public:
	// 3D array that stores the data
	std::vector<std::vector<std::vector<unsigned char>>> cells;

	// Constructor
	DensityMap(int dim);

	// Adds a line of data between p1 and p2
	void addLine(glm::vec3 p1, glm::vec3 p2, std::vector<unsigned char> vals);

	// Overwrites everything with value
	void clear(unsigned char value = 0);

	// Returns the vertices in a form useful to OpenGL
	std::vector<float> getVertexPositions();

	// Returns the cell densities
	std::vector<unsigned char> getVertexDensities();

	// Returns dim
	int getDim();
};

// Not being used right now, but maybe in the future
// to get smoother shading
float pointLineDistance(glm::vec3 a, glm::vec3 b, glm::vec3 v);