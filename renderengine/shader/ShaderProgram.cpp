#include "ShaderProgram.h"

#include <GL/glew.h>
#include <iostream>

#include "../utils/IOUtils.h"

void ShaderProgram::init() {
    AShaderProgram::create();
}

void ShaderProgram::createVertexShader(const std::string &fileName) {
    std::string code;
    IOUtils::readAllLines(fileName, &code);
    vertexShaderId = createShader(code, GL_VERTEX_SHADER);
}

void ShaderProgram::createFragmentShader(const std::string &fileName) {
    std::string code;
    IOUtils::readAllLines(fileName, &code);
    fragmentShaderId = createShader(code, GL_FRAGMENT_SHADER);
}

void ShaderProgram::link() {
    glLinkProgram(programId);

    GLint status;
    glGetProgramiv(programId, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        int length;
        char message[1024];
        glGetProgramInfoLog(programId, 1024, &length, message);
        std::string err = "Error linking shader: " + std::string(message);
        std::cerr << err << std::endl;
//        throw std::runtime_error(err.c_str());
    }

    glValidateProgram(programId);
    glGetProgramiv(programId, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE) {
        int length;
        char message[1024];
        glGetProgramInfoLog(programId, 1024, &length, message);
        std::string err = "Error validating shader: " + std::string(message);
        std::cerr << err << std::endl;
//        throw std::runtime_error(err.c_str());
    }
}
