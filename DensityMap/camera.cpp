#include "camera.h"

Camera::Camera() {
	deltaTime = 0.0;
	lastFrame = 0.0;

	pitch = 0.0;
	yaw = -90.0;
	fov = 70.0;
	sensitivity = 0.1;
	speed = 2.5;

	position = { 0.0,  0.0,  15.0 };
	front    = { 0.0,  0.0,  0.0 };
	up       = { 0.0,  1.0,  0.0 };
	worldUp  = { 0.0,  1.0,  0.0 };
	right    = { 1.0,  0.0,  0.0 };

	updateVectors();
}

void Camera::processMouseMovement(double xoffset, double yoffset) {
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0)
		pitch = 89.0;
	if (pitch < -89.0)
		pitch = -89.0;

	updateVectors();
}

void Camera::processKeyboard(Direction direction, double sprinting) {
	double velocity = speed * deltaTime;

	if (sprinting) velocity *= 3;

	if (direction == FORWARD)
		position += front * velocity;
	if (direction == BACKWARD)
		position -= front * velocity;
	if (direction == RIGHT)
		position += right * velocity;
	if (direction == LEFT)
		position -= right * velocity;
	if (direction == UP)
		position += up * velocity;
	if (direction == DOWN)
		position -= up * velocity;
}

void Camera::updateVectors() {
	glm::dvec3 tempFront;
	tempFront.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	tempFront.y = sin(glm::radians(pitch));
	tempFront.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	tempFront = glm::normalize(tempFront);

	right = glm::normalize(glm::cross(tempFront, worldUp));
	up = glm::normalize(glm::cross(right, tempFront));
	front = tempFront;
}

void Camera::zoom(double yoffset) {
	fov -= yoffset;

//	if (fov <= 1.0)
//		fov = 1.0;
//	if (fov >= 70.0)
//		fov = 70.0;

}

glm::dmat4 Camera::getViewMatrix() {
	return glm::lookAt(position, position + front, up);
}




