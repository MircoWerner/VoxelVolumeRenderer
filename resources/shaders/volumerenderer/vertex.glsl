/**
 * Vertex shader.
 *
 * @author Mirco Werner
 */
#version 430 core

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec2 TEXTURE;

out vec2 tex;

void main() {
    gl_Position = vec4(POSITION, 1.0);
    tex = TEXTURE;
}