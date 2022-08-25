/**
 * Shader to render the coordinate system.
 *
 * @author Mirco Werner
 */
#version 430 core

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 COLOR;

uniform mat4 MVP;

out vec3 color;

void main() {
    gl_Position = MVP * vec4(POSITION, 1.0);

    color = COLOR;
}