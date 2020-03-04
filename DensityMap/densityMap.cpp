#include "densityMap.h"

DensityMap::DensityMap(long long int dim) {
	this->dim = dim;
	threshold = 0;

	std::string vCells =
		"// VERTEX SHADER						\n"
		"										\n"
		"#version 330 core						\n"
		"										\n"
		"uniform int dim;						\n"
		"										\n"
		"void main() {							\n"
		"	int x = gl_VertexID / (dim * dim);	\n"
		"	int y = (gl_VertexID / dim) % dim;	\n"
		"	int z = gl_VertexID % dim;			\n"
		"										\n"
		"	gl_Position = vec4(x, y, z, 1.0);	\n"
		"}										\n";

	std::string fCells =
		"// FRAGMENT SHADER												\n"
		"																\n"
		"#version 330 core												\n"
		"																\n"
		"out vec4 FragColor;											\n"
		"																\n"
		"in float fShade;												\n"
		"																\n"
		"void main() {													\n"
		"	float shade = fShade * fShade * fShade * fShade * fShade;	\n"
		"	shade = clamp(shade, 0.003, 1.0);							\n"
		"	FragColor = vec4(1.0, 1.0, 1.0, shade);						\n"
		"}																\n";

	std::string gCells =
		"// GEOMETRY SHADER																\n"
		"																				\n"
		"#version 330 core																\n"
		"																				\n"
		"layout(points) in;																\n"
		"layout(triangle_strip, max_vertices = 12) out;									\n"
		"																				\n"
		"out float fShade;																\n"
		"																				\n"
		"uniform mat4 projection;														\n"
		"uniform mat4 view;																\n"
		"uniform mat4 model;															\n"
		"																				\n"
		"uniform int dim;																\n"
		"uniform float threshold;														\n"
		"																				\n"
		"uniform samplerBuffer densities;												\n"
		"																				\n"
		"vec4 transform(float x, float y, float z) {									\n"
		"	return projection * view * model * vec4(x, y, z, 1.0);						\n"
		"}																				\n"
		"																				\n"
		"float getDensity(int x, int y, int z) {										\n"
		"	return texelFetch(densities, x * dim * dim + y * dim + z).x;				\n"
		"}																				\n"
		"																				\n"
		"void genSquare(int x, int y, int z, int a, int b, int c) {						\n"
		"	gl_Position = transform(x, y, z);											\n"
		"	fShade = getDensity(x, y, z);												\n"
		"	EmitVertex();																\n"
		"																				\n"
		"	for (int i = 0; i < 3; i++) {												\n"
		"		if (ivec3(a, b, c)[i] == 1) {											\n"
		"			ivec3 add = ivec3(0);												\n"
		"			add[i] = 1;															\n"
		"																				\n"
		"			gl_Position = transform(x + add.x, y + add.y, z + add.z);			\n"
		"			fShade = getDensity(x + add.x, y + add.y, z + add.z);				\n"
		"			EmitVertex();														\n"
		"		}																		\n"
		"	}																			\n"
		"																				\n"
		"	gl_Position = transform(x + a, y + b, z + c);								\n"
		"	fShade = getDensity(x + a, y + b, z + c);									\n"
		"	EmitVertex();																\n"
		"																				\n"
		"	EndPrimitive();																\n"
		"}																				\n"
		"																				\n"
		"void main() {																	\n"
		"	int x = int(gl_in[0].gl_Position.x);										\n"
		"	int y = int(gl_in[0].gl_Position.y);										\n"
		"	int z = int(gl_in[0].gl_Position.z);										\n"
		"																				\n"
		"	if (getDensity(x, y, z) < threshold) {										\n"
		"		return;																	\n"
		"	}																			\n"
		"																				\n"
		"	if (x != dim - 1 && y != dim - 1) {											\n"
		"		genSquare(x, y, z, 1, 1, 0);											\n"
		"	}																			\n"
		"																				\n"
		"	if (x != dim - 1 && z != dim - 1) {											\n"
		"		genSquare(x, y, z, 1, 0, 1);											\n"
		"	}																			\n"
		"																				\n"
		"	if (y != dim - 1 && z != dim - 1) {											\n"
		"		genSquare(x, y, z, 0, 1, 1);											\n"
		"	}																			\n"
		"}																				\n";

	std::string vLines =
		"// VERTEX SHADER												\n"
		"																\n"
		"#version 330 core												\n"
		"																\n"
		"layout(location = 0) in vec3 aPos;								\n"
		"																\n"
		"uniform mat4 projection;										\n"
		"uniform mat4 view;												\n"
		"uniform mat4 model;											\n"
		"																\n"
		"void main() {													\n"
		"	gl_Position = projection * view * model * vec4(aPos, 1.0);	\n"
		"}																\n";

	std::string fLines =
		"// FRAGMENT SHADER			\n"
		"							\n"
		"#version 330 core			\n"
		"							\n"
		"out vec4 FragColor;		\n"
		"							\n"
		"void main() {				\n"
		"	FragColor = vec4(1.0);	\n"
		"}							\n";

	cellShader = Shader(vCells.c_str(), fCells.c_str(), gCells.c_str(), false);
	lineShader = Shader(vLines.c_str(), fLines.c_str(), false);

	unsigned char* tempCells = new unsigned char[dim * dim * dim];

	// Initializing the array and filling it with zeroes
	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			for (int k = 0; k < dim; k++) {
				tempCells[i * dim * dim + j * dim + k] = 0;
			}
		}
	}

	// Allows blending (translucent drawing)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Creating buffers on the graphics card

	// ------------------
	// Buffer containing the densities of each cell
	glGenBuffers(1, &cellDensityTBO);
	glGenVertexArrays(1, &cellVAO);

	glBindVertexArray(cellVAO);

	glBindBuffer(GL_TEXTURE_BUFFER, cellDensityTBO);
	glBufferData(GL_TEXTURE_BUFFER, dim * dim * dim * sizeof(unsigned char), tempCells, GL_STATIC_DRAW);

	// Associates the texture buffer with the array we just made

	glGenTextures(1, &cellDensityBufferTexture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, cellDensityBufferTexture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R8, cellDensityTBO);

	delete[] tempCells;

	glBindBuffer(GL_TEXTURE_BUFFER, cellDensityTBO);
	cells = (unsigned char*)glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);

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
	// Fills the whole array with value
	// Defaults to zero

	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			for (int k = 0; k < dim; k++) {
				write(i, j, k, value);
			}
		}
	}
}

void DensityMap::writeQueuesToGPU() {
	// Keeps the queues thread-safe
	std::lock_guard<std::mutex> lock(mutex);

	int lineQueueSize = lineQueue.size();
	for (int l = 0; l < lineQueueSize; l++) {
		// Getting the line from the front of the queue
		Line line = lineQueue.front();
		lineQueue.pop();

		glm::vec3 p1 = line.p1;
		glm::vec3 p2 = line.p2;
		std::vector<unsigned char> vals = line.vals;

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
			int ix = x * (dim - 1);
			int iy = y * (dim - 1);
			int iz = z * (dim - 1);

			// Get the next value from the vals array
			newValue += vals[i];
			numNewValues++;

			if (ix != px || iy != py || iz != pz) {
				// Write the new value to the array
				cells[ix * dim * dim + iy * dim + iz] = static_cast<unsigned char>(newValue / numNewValues);

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

	int cellQueueSize = cellQueue.size();
	for (int c = 0; c < cellQueueSize; c++) {
		// Getting the cell from the front of the queue
		Cell cell = cellQueue.front();
		cellQueue.pop();

		cells[cell.x * dim * dim + cell.y * dim + cell.z] = cell.value;
	}

	// The thread lock automatically releases in its destructor
}

// Returns dim
int DensityMap::getDim() {
	return dim;
}

void DensityMap::draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model) {
	glBindBuffer(GL_TEXTURE_BUFFER, cellDensityTBO);
	glUnmapBuffer(GL_TEXTURE_BUFFER);

	// Needed to standardize the size of the grid
	glm::mat4 _model = glm::scale<float>(glm::mat4(1.0), glm::vec3(10.0 / (dim - 1), 10.0 / (dim - 1), 10.0 / (dim - 1)));
	_model = glm::translate<float>(_model, glm::vec3(-(dim - 1) / 2.0, -(dim - 1) / 2.0, -(dim - 1) / 2.0));

	// Drawing the volume map
	cellShader.use();
	cellShader.setMat4("projection", projection);
	cellShader.setMat4("view", view);
	cellShader.setMat4("model", model * _model);
	cellShader.setInt("dim", dim);
	cellShader.setInt("densities", 0);
	cellShader.setFloat("threshold", static_cast<float>(threshold) / 255);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, cellDensityBufferTexture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R8, cellDensityTBO);

	glBindVertexArray(cellVAO);
	glDrawArrays(GL_POINTS, 0, dim * dim * dim);

	// Drawing the white lines
	lineShader.use();
	lineShader.setMat4("projection", projection);
	lineShader.setMat4("view", view);
	lineShader.setMat4("model", model);

	glBindVertexArray(lineVAO);
	glDrawArrays(GL_LINES, 0, 24);

	cells = (unsigned char*)glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);

	writeQueuesToGPU();
}

void DensityMap::setThreshold(unsigned char value) {
	threshold = value;
}

unsigned char DensityMap::getThreshold() {
	return threshold;
}

void DensityMap::addLine(glm::vec3 p1, glm::vec3 p2, std::vector<unsigned char> vals) {
	std::lock_guard<std::mutex> lock(mutex);
	lineQueue.push(Line(p1, p2, vals));
}

void DensityMap::write(unsigned int x, unsigned int y, unsigned int z, unsigned char value) {
	std::lock_guard<std::mutex> lock(mutex);
	cellQueue.push(Cell(x, y, z, value));
}