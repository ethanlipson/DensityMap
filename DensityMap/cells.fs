// FRAGMENT SHADER

#version 330 core

out vec4 FragColor;

in float fShade;
void main() {
	float shade = fShade * fShade * fShade * fShade * fShade;
    shade = clamp(shade, 0.0025, 1.0);
    FragColor = vec4(1.0, 1.0, 1.0, shade);
}