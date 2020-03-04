#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

#include <vector>
#include <queue>
#include <mutex>

// Class that stores the density readings
// and other related info
class DensityMap {
private:
	// Struct for storing data in the lineQueue
	struct Line {
		glm::vec3 p1;
		glm::vec3 p2;

		std::vector<unsigned char> vals;

		Line(glm::vec3 p1, glm::vec3 p2, std::vector<unsigned char> vals) {
			this->p1 = p1;
			this->p2 = p2;
			this->vals = vals;
		}
	};

	struct Cell {
		unsigned int x;
		unsigned int y;
		unsigned int z;

		unsigned char value;

		Cell(unsigned int x, unsigned int y, unsigned int z, unsigned char value) {
			this->x = x;
			this->y = y;
			this->z = z;
			this->value = value;
		}
	};

	// Queue for storing lines queued by addLine()
	std::queue<Line> lineQueue;

	// Queue for storing cells queued by write()
	std::queue<Cell> cellQueue;

	// Necessary for thread-safety
	std::mutex mutex;

	// Pointer to the cells on the graphics card
	unsigned char* cells;
	
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

	// Writes cells and lines in both queues to the GPU
	void writeQueuesToGPU();

public:
	// Constructor
	DensityMap(long long int dim);

	// Overwrites everything with value
	void clear(unsigned char value = 0);

	// Returns dim
	int getDim();

	// Draws to the screen and optionally clears the screen
	void draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model);

	// Adds a line of data between p1 and p2 to the lineQueue
	void addLine(glm::vec3 p1, glm::vec3 p2, std::vector<unsigned char> vals);

	// Set and get the threshold for drawing a cell
	void setThreshold(unsigned char value);
	unsigned char getThreshold();

	// Writes to one cell of the density map
	void write(unsigned int x, unsigned int y, unsigned int z, unsigned char value);
};