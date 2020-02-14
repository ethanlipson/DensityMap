#include "densityMap.h"

DensityMap::DensityMap(int dim) {
	this->dim = dim;
	threshold = 0;

	std::string vCells =
		"// VERTEX SHADER											  \n"
		"															  \n"
		"#version 330 core											  \n"
		"															  \n"
		"layout(location = 0) in vec3 aPos;							  \n"
		"layout(location = 1) in uint aShade;						  \n"
		"															  \n"
		"out float fShade;											  \n"
		"															  \n"
		"uniform mat4 projection;									  \n"
		"uniform mat4 view;											  \n"
		"uniform mat4 model;										  \n"
		"															  \n"
		"void main() {												  \n"
		"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"	fShade = float(aShade) / 255.0;							  \n"
		"}															  \n";

	std::string fCells =
		"// FRAGMENT SHADER											 \n"
		"															 \n"
		"#version 330 core											 \n"
		"															 \n"
		"out vec4 FragColor;										 \n"
		"															 \n"
		"in float fShade;											 \n"
		"															 \n"
		"void main() {												 \n"
		"	float shade = fShade * fShade * fShade * fShade * fShade;\n"
		"	shade = clamp(shade, 0.0025, 1.0);						 \n"
		"	FragColor = vec4(1.0, 1.0, 1.0, shade);					 \n"
		"}															 \n";

	std::string vLines =
		"// VERTEX SHADER											  \n"
		"															  \n"
		"#version 330 core											  \n"
		"															  \n"
		"layout(location = 0) in vec3 aPos;							  \n"
		"															  \n"
		"uniform mat4 projection;									  \n"
		"uniform mat4 view;											  \n"
		"uniform mat4 model;										  \n"
		"															  \n"
		"void main() {												  \n"
		"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"}															  \n";

	std::string fLines =
		"// FRAGMENT SHADER		  \n"
		"						  \n"
		"#version 330 core		  \n"
		"						  \n"
		"out vec4 FragColor;	  \n"
		"						  \n"
		"void main() {			  \n"
		"	FragColor = vec4(1.0);\n"
		"}						  \n";

	cellShader = Shader(vCells.c_str(), fCells.c_str(), false);
	lineShader = Shader(vLines.c_str(), fLines.c_str(), false);

	// Initializing the array and filling it with zeroes
	for (int i = 0; i < dim; i++) {
		cells.push_back(std::vector<std::vector<unsigned char>>{});

		for (int j = 0; j < dim; j++) {
			cells.back().push_back(std::vector<unsigned char>{});

			for (int k = 0; k < dim; k++) {
				cells.back().back().push_back(0);
			}
		}
	}

	// Allows blending (translucent drawing)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Gets the vertices in a form usable to OpenGL
	std::vector<float> cellPositions = getVertexPositions();
	std::vector<unsigned char> cellDensities = getVertexDensities();

	// Creating buffers on the graphics card
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

	// ------------------
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
		5,  5, -5,  5,  5,  5
	};

	// Initializing the buffer storing the vertices
	// of the white lines on the graphics card
	glGenBuffers(1, &lineVBO);
	glGenVertexArrays(1, &lineVAO);

	glBindVertexArray(lineVAO);

	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
}

void DensityMap::clear(unsigned char value) {
	// Fills the whole array with zeroes

	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			for (int k = 0; k < dim; k++) {
				cells[i][j][k] = value;
			}
		}
	}
}

void DensityMap::addLine(glm::vec3 p1, glm::vec3 p2, std::vector<unsigned char> vals) {
	int numVals = vals.size();

	// x, y, and z coordinates of the current data point
	// Moves along the line defined by p1 and p2
	float x = p1.x;
	float y = p1.y;
	float z = p1.z;

	// Direction of the line defined by p1 and p2
	float dx = (p2.x - p1.x) / numVals;
	float dy = (p2.y - p1.y) / numVals;
	float dz = (p2.z - p1.z) / numVals;

	// Multiple values can fall in the same box, so we
	// take their average and them combine it with
	// the current value in the box
	int newValue = 0;
	int numNewValues = 0;

	// Previous ix, iy, and iz values
	int px = -1;
	int py = -1;
	int pz = -1;

	for (int i = 0; i < numVals; i++) {
		// Cell indices determined by x, y, and z
		int ix = x * dim;
		int iy = y * dim;
		int iz = z * dim;

		// Get the next value from the vals array
		newValue += vals[i];
		numNewValues++;

		if (ix != px || iy != py || iz != pz) {
			// Write the new value to the array
			cells[ix][iy][iz] = static_cast<unsigned char>(newValue / numNewValues);

			// Reset these values (since we are in a new cell now)
			newValue = 0;
			numNewValues = 0;
		}

		// Move x, y, and z along the line
		x += dx;
		y += dy;
		z += dz;

		// Update the previous ix, iy, and iz
		px = ix;
		py = iy;
		pz = iz;
	}
}

// Returns the vertices in a form useful to OpenGL
std::vector<float> DensityMap::getVertexPositions() {
	std::vector<float> vertices;

	numVertices = 0;

	for (int i = 0; i < dim - 1; i++) {
		for (int j = 0; j < dim - 1; j++) {
			for (int k = 0; k < dim; k++) {
				float v1[3] = { static_cast<float>(i), static_cast<float>(j), static_cast<float>(k) };
				float v2[3] = { static_cast<float>(i) + 1, static_cast<float>(j), static_cast<float>(k) };
				float v3[3] = { static_cast<float>(i), static_cast<float>(j) + 1, static_cast<float>(k) };
				float v4[3] = { static_cast<float>(i) + 1, static_cast<float>(j) + 1, static_cast<float>(k) };

				if (cells[i][j][k] < threshold && cells[i + 1][j][k] < threshold && 
					cells[i][j + 1][k] < threshold && cells[i + 1][j + 1][k] < threshold) {
					continue;
				}

				vertices.insert(vertices.end(), v1, v1 + 3);
				vertices.insert(vertices.end(), v2, v2 + 3);
				vertices.insert(vertices.end(), v4, v4 + 3);

				vertices.insert(vertices.end(), v1, v1 + 3);
				vertices.insert(vertices.end(), v3, v3 + 3);
				vertices.insert(vertices.end(), v4, v4 + 3);

				numVertices += 6;
			}
		}
	}

	for (int i = 0; i < dim - 1; i++) {
		for (int j = 0; j < dim; j++) {
			for (int k = 0; k < dim - 1; k++) {
                float v1[3] = { static_cast<float>(i), static_cast<float>(j), static_cast<float>(k) };
                float v2[3] = { static_cast<float>(i) + 1, static_cast<float>(j), static_cast<float>(k) };
                float v3[3] = { static_cast<float>(i), static_cast<float>(j), static_cast<float>(k) + 1 };
                float v4[3] = { static_cast<float>(i) + 1, static_cast<float>(j), static_cast<float>(k) + 1 };

				if (cells[i][j][k] < threshold && cells[i + 1][j][k] < threshold && 
					cells[i][j][k + 1] < threshold && cells[i + 1][j][k + 1] < threshold) {
					continue;
				}

				vertices.insert(vertices.end(), v1, v1 + 3);
				vertices.insert(vertices.end(), v2, v2 + 3);
				vertices.insert(vertices.end(), v4, v4 + 3);

				vertices.insert(vertices.end(), v1, v1 + 3);
				vertices.insert(vertices.end(), v3, v3 + 3);
				vertices.insert(vertices.end(), v4, v4 + 3);

				numVertices += 6;
			}
		}
	}

	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim - 1; j++) {
			for (int k = 0; k < dim - 1; k++) {
                float v1[3] = { static_cast<float>(i), static_cast<float>(j), static_cast<float>(k) };
                float v2[3] = { static_cast<float>(i), static_cast<float>(j) + 1, static_cast<float>(k) };
                float v3[3] = { static_cast<float>(i), static_cast<float>(j), static_cast<float>(k) + 1 };
                float v4[3] = { static_cast<float>(i), static_cast<float>(j) + 1, static_cast<float>(k) + 1};

				if (cells[i][j][k] < threshold && cells[i][j + 1][k] < threshold && 
					cells[i][j][k + 1] < threshold && cells[i][j + 1][k + 1] < threshold) {
					continue;
				}

				vertices.insert(vertices.end(), v1, v1 + 3);
				vertices.insert(vertices.end(), v2, v2 + 3);
				vertices.insert(vertices.end(), v4, v4 + 3);

				vertices.insert(vertices.end(), v1, v1 + 3);
				vertices.insert(vertices.end(), v3, v3 + 3);
				vertices.insert(vertices.end(), v4, v4 + 3);

				numVertices += 6;
			}
		}
	}

	return vertices;
}

// Returns the cell densities
std::vector<unsigned char> DensityMap::getVertexDensities() {
	std::vector<unsigned char> densities;

	numVertices = 0;

	for (int i = 0; i < dim - 1; i++) {
		for (int j = 0; j < dim - 1; j++) {
			for (int k = 0; k < dim; k++) {
				unsigned char d1 = cells[i][j][k];
				unsigned char d2 = cells[i + 1][j][k];
				unsigned char d3 = cells[i][j + 1][k];
				unsigned char d4 = cells[i + 1][j + 1][k];

				if (d1 < threshold && d2 < threshold && d3 < threshold && d4 < threshold) {
					continue;
				}

				densities.push_back(d1);
				densities.push_back(d2);
				densities.push_back(d4);

				densities.push_back(d1);
				densities.push_back(d3);
				densities.push_back(d4);

				numVertices += 6;
			}
		}
	}

	for (int i = 0; i < dim - 1; i++) {
		for (int j = 0; j < dim; j++) {
			for (int k = 0; k < dim - 1; k++) {
				unsigned char d1 = cells[i][j][k];
				unsigned char d2 = cells[i + 1][j][k];
				unsigned char d3 = cells[i][j][k + 1];
				unsigned char d4 = cells[i + 1][j][k + 1];

				if (d1 < threshold && d2 < threshold && d3 < threshold && d4 < threshold) {
					continue;
				}

				densities.push_back(d1);
				densities.push_back(d2);
				densities.push_back(d4);

				densities.push_back(d1);
				densities.push_back(d3);
				densities.push_back(d4);

				numVertices += 6;
			}
		}
	}

	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim - 1; j++) {
			for (int k = 0; k < dim - 1; k++) {
				unsigned char d1 = cells[i][j][k];
				unsigned char d2 = cells[i][j + 1][k];
				unsigned char d3 = cells[i][j][k + 1];
				unsigned char d4 = cells[i][j + 1][k + 1];

				if (d1 < threshold && d2 < threshold && d3 < threshold && d4 < threshold) {
					continue;
				}

				densities.push_back(d1);
				densities.push_back(d2);
				densities.push_back(d4);

				densities.push_back(d1);
				densities.push_back(d3);
				densities.push_back(d4);

				numVertices += 6;
			}
		}
	}

	return densities;
}

// Returns dim
int DensityMap::getDim() {
	return dim;
}

void DensityMap::draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model) {
	// Needed to standardize the size of the grid
	glm::mat4 _model = glm::scale<float>(glm::mat4(1.0), glm::vec3(10.0 / (dim - 1), 10.0 / (dim - 1), 10.0 / (dim - 1)));
	_model = glm::translate<float>(_model, glm::vec3(-(dim - 1) / 2.0, -(dim - 1) / 2.0, -(dim - 1) / 2.0));

	// Drawing the volume map
	cellShader.use();
	cellShader.setMat4("projection", projection);
	cellShader.setMat4("view", view);
	cellShader.setMat4("model", model * _model);

	glBindVertexArray(cellVAO);
	glDrawArrays(GL_TRIANGLES, 0, numVertices);

	// Drawing the white lines
	lineShader.use();
	lineShader.setMat4("projection", projection);
	lineShader.setMat4("view", view);
	lineShader.setMat4("model", model);

	glBindVertexArray(lineVAO);
	glDrawArrays(GL_LINES, 0, 24);
}

void DensityMap::updateVertexBuffers() {
	// Gets the vertex positions
	std::vector<float> positions = getVertexPositions();

	// Writes the vertex positions to the vertex buffer on the graphics card
	glBindBuffer(GL_ARRAY_BUFFER, cellPositionVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(float), positions.data());

	// Gets the vertex densities
	std::vector<unsigned char> densities = getVertexDensities();

	// Writes the vertices to the vertex buffer on the graphics card
	glBindBuffer(GL_ARRAY_BUFFER, cellDensityVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, densities.size() * sizeof(unsigned char), densities.data());
}

void DensityMap::setThreshold(unsigned char value) {
	threshold = value;
}

unsigned char DensityMap::getThreshold() {
	return threshold;
}