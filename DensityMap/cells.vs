// VERTEX SHADER

#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in uint aShade;

out float fShade;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	fShade = float(aShade) / 255.0;
}