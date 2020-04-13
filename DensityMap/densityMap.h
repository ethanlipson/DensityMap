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
public:
	// Enum for writeLine()
	enum class WriteMode {
		Max,
		Avg
	};

	// Constructor
	DensityMap(long long int dim);

	// Overwrites everything with value
	void clear(unsigned char value = 0);

	// Returns dim
	int getDim();

	// Draws to the screen and optionally clears the screen
	void draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model);

	// Adds a line of data between p1 and p2 to the lineQueue
	void writeLine(glm::vec3 p1, glm::vec3 p2, std::vector<unsigned char> vals, WriteMode writeMode = WriteMode::Avg);

	// Writes to one cell of the density map
	void writeCell(unsigned int x, unsigned int y, unsigned int z, unsigned char value);

	// Gets the value at a specific index in the array and writes it to val
	unsigned char readCell(int x, int y, int z);

	// Returns the value at a specific position in the array (interpolated)
	// x, y, and z must all be on the half-open range [0, 1)
	unsigned char readCellInterpolated(float x, float y, float z);

	// Gets the values along the line between two points and writes them to a given array
	// using readCellInterpolated() several times
	void readLine(float x, float, float z, int numVals, unsigned char* vals);

	// Set and get the threshold for drawing a cell
	void setThreshold(unsigned char value);
	unsigned char getThreshold();

	// Set and get the image brightness
	void setBrightness(float value);
	float getBrightness();

	// Set and get the image contrast
	void setContrast(float value);
	float getContrast();

	// Set and get the update coefficient used in writeLine()
	void setUpdateCoefficient(float value);
	float getUpdateCoefficient();

private:
	// Structs for writing data
	struct LineWrite {
		glm::vec3 p1;
		glm::vec3 p2;

		std::vector<unsigned char> vals;

		WriteMode writeMode;

		LineWrite(glm::vec3 p1, glm::vec3 p2, std::vector<unsigned char> vals, WriteMode writeMode) {
			this->p1 = p1;
			this->p2 = p2;
			this->vals = vals;
			this->writeMode = writeMode;
		}
	};

	struct CellWrite {
		unsigned int x;
		unsigned int y;
		unsigned int z;

		unsigned char value;

		CellWrite(unsigned int x, unsigned int y, unsigned int z, unsigned char value) {
			this->x = x;
			this->y = y;
			this->z = z;
			this->value = value;
		}
	};

	// Queues for storing write requests
	std::queue<LineWrite> lineWriteQueue;
	std::queue<CellWrite> cellWriteQueue;

	// Necessary for thread-safety
	std::mutex writeMutex;
	std::mutex readMutex;

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

	// Values that determine how the image is drawn
	unsigned char threshold;
	float brightness;
	float contrast;

	// The weight for the weighted average taken in writeLine()
	float updateCoefficient;

	// Creating the shaders for the cells in the cube
	// and for the lines of the border of the cube
	Shader cellShader;
	Shader lineShader;

	// Resolves all write requests in the queues
	void resolveQueues();

	// Gets the value of a specific cell in the array
	unsigned char getCell(int x, int y, int z);
};