/**
 * Shader to render the coordinate system.
 *
 * @author Mirco Werner
 */
#version 430 core

in vec3 color;

out vec4 fragColor;

void main() {
    fragColor = vec4(color, 1.0);
}