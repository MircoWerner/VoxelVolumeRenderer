#include "MipMap.h"

#include <iostream>
#include <chrono>

void MipMap::init(GLenum glInternalFormat, const std::string &shader) {
    m_glInternalFormat = glInternalFormat;

    m_computeShaderProgram.init();
    m_computeShaderProgram.createComputeShader("../resources/shaders/volumerenderer/utils/" + shader + ".glsl");
    m_computeShaderProgram.link();
    m_computeShaderProgram.createUniform("outputDimension");
    m_computeShaderProgram.createUniform("inputDimension");
    ShaderProgram::unbind();
}

void MipMap::execute(GLuint textureId, glm::ivec3 inputDimension, int levels) {
    m_computeShaderProgram.bind();

    m_computeShaderProgram.setUniform("inputTexture", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, textureId);

    glm::ivec3 outputDimension = inputDimension;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 1; i <= levels; i++) {
        m_computeShaderProgram.setUniform("inputDimension", outputDimension);

        outputDimension.x = glm::max(1, (outputDimension.x / 2));
        outputDimension.y = glm::max(1, (outputDimension.y / 2));
        outputDimension.z = glm::max(1, (outputDimension.z / 2));

        glBindImageTexture(0, textureId, i, false, 0, GL_WRITE_ONLY, m_glInternalFormat);

        m_computeShaderProgram.setUniform("outputDimension", outputDimension);
        m_computeShaderProgram.setUniform("inputLevel", i - 1);

        m_computeShaderProgram.dispatch(outputDimension.x, outputDimension.y, outputDimension.z);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glFinish();
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "[MIPMAP_Compute_Shader] Execution time = "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
              << "[µs]" << std::endl;

    glBindTexture(GL_TEXTURE_3D, 0);

    glBindImageTexture(0, 0, 0, false, 0, GL_WRITE_ONLY, m_glInternalFormat);

    ComputeShaderProgram::unbind();
}

void MipMap::cleanUp() {
    m_computeShaderProgram.cleanUp();
}
