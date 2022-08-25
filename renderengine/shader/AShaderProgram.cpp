#include "AShaderProgram.h"

#include <GL/glew.h>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

void AShaderProgram::create() {
    programId = glCreateProgram();
    if (programId == 0) {
        throw std::runtime_error("Error creating shader program.");
    }
}

int AShaderProgram::createShader(const std::string &shaderCode, int shaderType) const {
    int shaderId = glCreateShader(shaderType);
    if (shaderId == 0) {
        std::string err = "Error creating shader: " + shaderType;
        throw std::runtime_error(err.c_str());
    }

    const char *code = shaderCode.c_str();
    glShaderSource(shaderId, 1, &code, nullptr);
    glCompileShader(shaderId);

    int status;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        int length;
        char message[1024];
        glGetShaderInfoLog(shaderId, 1024, &length, message);
        std::string err = "Error compiling shader: " + std::string(message);
        std::cerr << err << std::endl;
//        throw std::runtime_error(err.c_str());
    }

    glAttachShader(programId, shaderId);

    return shaderId;
}

void AShaderProgram::bind() const {
    glUseProgram(programId);
}

void AShaderProgram::unbind() {
    glUseProgram(0);
}

void AShaderProgram::cleanUp() const {
    unbind();
    if (programId != 0) {
        glDeleteProgram(programId);
    }
}

void AShaderProgram::createUniform(const std::string &uniformName) {
    int uniformLocation = glGetUniformLocation(programId, uniformName.c_str());
    if (uniformLocation == -1) {
        //        std::string err = "Error creating shader uniform.";
        //        throw std::runtime_error(err.c_str());
        std::cerr << "Error creating shader (" << programId << ") uniform: " << uniformName << std::endl;
    }
    uniforms[uniformName] = uniformLocation;
}

void AShaderProgram::setUniform(const std::string &uniformName, glm::mat4 value) {
    glUniformMatrix4fv(uniforms[uniformName], 1, false, glm::value_ptr(value));
}

void AShaderProgram::setUniform(const std::string &uniformName, int value) {
    glUniform1i(uniforms[uniformName], value);
}

void AShaderProgram::setUniform(const std::string &uniformName, float value) {
    glUniform1f(uniforms[uniformName], value);
}

void AShaderProgram::setUniform(const std::string &uniformName, glm::vec3 value) {
    glUniform3f(uniforms[uniformName], value.x, value.y, value.z);
}

void AShaderProgram::setUniform(const std::string &uniformName, glm::ivec3 value) {
    glUniform3i(uniforms[uniformName], value.x, value.y, value.z);
}

void AShaderProgram::setUniform(const std::string &uniformName, glm::vec4 value) {
    glUniform4f(uniforms[uniformName], value.x, value.y, value.z, value.w);
}

int AShaderProgram::getUniformLocation(const std::string &uniformName) {
    return uniforms[uniformName];
}
