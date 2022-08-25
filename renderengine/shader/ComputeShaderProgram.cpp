#include "ComputeShaderProgram.h"

#include <iostream>

#include "../utils/IOUtils.h"

void ComputeShaderProgram::init() {
    AShaderProgram::create();
}

void ComputeShaderProgram::createComputeShader(const std::string &fileName) {
    std::string code;
    IOUtils::readAllLines(fileName, &code);
    computeShaderId = createShader(code, GL_COMPUTE_SHADER);
}

void ComputeShaderProgram::link() {
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

    glGetProgramiv(programId, GL_COMPUTE_WORK_GROUP_SIZE, m_localWorkGroupSize);
}

unsigned int nextPowerOf2(unsigned int n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

void ComputeShaderProgram::dispatch(int globalWorkSizeX, int globalWorkSizeY, int globalWorkSizeZ) {
    glDispatchCompute(nextPowerOf2(globalWorkSizeX) / m_localWorkGroupSize[0], nextPowerOf2(globalWorkSizeY) / m_localWorkGroupSize[1],
                      nextPowerOf2(globalWorkSizeZ) / m_localWorkGroupSize[2]);
}
