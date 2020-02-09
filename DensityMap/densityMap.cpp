#include "densityMap.h"

DensityMap::DensityMap(int dim) {
	this->dim = dim;

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

	for (int i = 0; i < numVals; i++) {
		// Cell index determined by x, y, and z
		int ix = x * dim;
		int iy = y * dim;
		int iz = z * dim;

		// Put the value in the array
		cells[ix][iy][iz] = vals[i];

		// Move x, y, and z along the line
		x += dx;
		y += dy;
		z += dz;
	}
}

// Returns the vertices in a form useful to OpenGL
std::vector<float> DensityMap::getVertexPositions() {
	std::vector<float> vertices;

	for (int i = 0; i < dim - 1; i++) {
		for (int j = 0; j < dim - 1; j++) {
			for (int k = 0; k < dim; k++) {
				float v1[3] = { static_cast<float>(i), static_cast<float>(j), static_cast<float>(k) };
				float v2[3] = { static_cast<float>(i) + 1, static_cast<float>(j), static_cast<float>(k) };
				float v3[3] = { static_cast<float>(i), static_cast<float>(j) + 1, static_cast<float>(k) };
				float v4[3] = { static_cast<float>(i) + 1, static_cast<float>(j) + 1, static_cast<float>(k) };

				vertices.insert(vertices.end(), v1, v1 + 3);
				vertices.insert(vertices.end(), v2, v2 + 3);
				vertices.insert(vertices.end(), v4, v4 + 3);

				vertices.insert(vertices.end(), v1, v1 + 3);
				vertices.insert(vertices.end(), v3, v3 + 3);
				vertices.insert(vertices.end(), v4, v4 + 3);
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

				vertices.insert(vertices.end(), v1, v1 + 3);
				vertices.insert(vertices.end(), v2, v2 + 3);
				vertices.insert(vertices.end(), v4, v4 + 3);

				vertices.insert(vertices.end(), v1, v1 + 3);
				vertices.insert(vertices.end(), v3, v3 + 3);
				vertices.insert(vertices.end(), v4, v4 + 3);
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

				vertices.insert(vertices.end(), v1, v1 + 3);
				vertices.insert(vertices.end(), v2, v2 + 3);
				vertices.insert(vertices.end(), v4, v4 + 3);

				vertices.insert(vertices.end(), v1, v1 + 3);
				vertices.insert(vertices.end(), v3, v3 + 3);
				vertices.insert(vertices.end(), v4, v4 + 3);
			}
		}
	}

	return vertices;
}

// Returns the cell densities
std::vector<unsigned char> DensityMap::getVertexDensities() {
	std::vector<unsigned char> densities;

	for (int i = 0; i < dim - 1; i++) {
		for (int j = 0; j < dim - 1; j++) {
			for (int k = 0; k < dim; k++) {
				unsigned char d1 = cells[i][j][k];
				unsigned char d2 = cells[i + 1][j][k];
				unsigned char d3 = cells[i][j + 1][k];
				unsigned char d4 = cells[i + 1][j + 1][k];

				densities.push_back(d1);
				densities.push_back(d2);
				densities.push_back(d4);

				densities.push_back(d1);
				densities.push_back(d3);
				densities.push_back(d4);
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

				densities.push_back(d1);
				densities.push_back(d2);
				densities.push_back(d4);

				densities.push_back(d1);
				densities.push_back(d3);
				densities.push_back(d4);
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

				densities.push_back(d1);
				densities.push_back(d2);
				densities.push_back(d4);

				densities.push_back(d1);
				densities.push_back(d3);
				densities.push_back(d4);
			}
		}
	}

	return densities;
}

// Returns dim
int DensityMap::getDim() {
	return dim;
}

// Not being used right now, but maybe in the future
// to get smoother shading
float pointLineDistance(glm::vec3 a, glm::vec3 b, glm::vec3 v) {
	glm::vec3 ab = b - a;
	glm::vec3 av = v - a;

	if (glm::dot(av, ab) <= 0.0) {
		return glm::length(av);
	}

	glm::vec3 bv = v - b;

	if (glm::dot(bv, ab) >= 0.0) {
		return glm::length(bv);
	}

	return glm::length(glm::cross(ab, av)) / glm::length(ab);
}