#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

enum Direction {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera {
public:
	double deltaTime;
	double lastFrame;

	double pitch;
	double yaw;
	double fov;
	double sensitivity;
	double speed;

	glm::dvec3 position;
	glm::dvec3 prevPos;
	glm::dvec3 front;
	glm::dvec3 up;
	glm::dvec3 worldUp;
	glm::dvec3 right;

	Camera();
	void processMouseMovement(double xoffset, double yoffset);
	void processKeyboard(Direction direction, double sprinting);
	void updateVectors();
	void zoom(double yoffset);
	glm::dmat4 getViewMatrix();
};