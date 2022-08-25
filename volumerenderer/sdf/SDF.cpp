#include "SDF.h"

#include <iostream>
#include <chrono>

#include "../utils/BinaryFileRW.h"
#include "../utils/MipMap.h"

SDF::SDF(bool readFromFile, bool writeToFile, std::string cellsFrameXXX) :
        m_readFromFile(readFromFile), m_writeToFile(writeToFile), m_cellsFrameXXX(std::move(cellsFrameXXX)) {

}

void SDF::init() {
    m_computeShaderProgram.init();
    m_computeShaderProgram.createComputeShader("../resources/shaders/volumerenderer/df/sdf_r16f.glsl");
    m_computeShaderProgram.link();
    m_computeShaderProgram.createUniform("tex_volume");
    m_computeShaderProgram.createUniform("volumeDimension");
    m_computeShaderProgram.createUniform("displacement");
    ShaderProgram::unbind();
}

GLuint SDF::execute(GLuint volumeTextureId, glm::ivec3 volumeDimension) {
    char *data = new char[2 * volumeDimension.x * volumeDimension.y * volumeDimension.z];

    if (m_readFromFile &&
        BinaryFileRW::exists("../resources/volume/" + m_cellsFrameXXX + "-sdf-cityblock-approx.raw")) {
        BinaryFileRW::readBytes("../resources/volume/" + m_cellsFrameXXX + "-sdf-cityblock-approx.raw",
                                2 * volumeDimension.x * volumeDimension.y * volumeDimension.z,
                                data);
    } else {
        std::memset(data, 0, 2 * volumeDimension.x * volumeDimension.y * volumeDimension.z);

        GLuint computeSDFTextureId = 0;

        glGenTextures(1, &computeSDFTextureId);
        glBindTexture(GL_TEXTURE_3D, computeSDFTextureId);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, volumeDimension.x, volumeDimension.y, volumeDimension.z, 0,
                     GL_RED,
                     GL_HALF_FLOAT, data);
        glBindTexture(GL_TEXTURE_3D, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, volumeTextureId);

        glBindImageTexture(0, computeSDFTextureId, 0, false, 0, GL_READ_WRITE, GL_R16F);

        {
            m_computeShaderProgram.bind();
            m_computeShaderProgram.setUniform("tex_volume", 0);
            m_computeShaderProgram.setUniform("volumeDimension", volumeDimension);

            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            for (int i = 0; i < 512; i++) {
                const int dimension = 64;
                for (int z = 0; z < volumeDimension.z; z += dimension) {
                    for (int y = 0; y < volumeDimension.y; y += dimension) {
                        for (int x = 0; x < volumeDimension.x; x += dimension) {
                            m_computeShaderProgram.setUniform("displacement", glm::ivec3(x, y, z));
                            m_computeShaderProgram.dispatch(dimension, dimension, dimension);
                            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                        }
                    }
                }
                glFinish();
                std::cout << "SDF iteration: " << i << std::endl;
            }
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "[SDF_Compute_Shader] Execution time = "
                      << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                      << "[µs]" << std::endl;
        }

        glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_R16F);

        glBindTexture(GL_TEXTURE_3D, 0);

        ComputeShaderProgram::unbind();

        glGetTextureImage(computeSDFTextureId, 0, GL_RED, GL_HALF_FLOAT,
                          2 * volumeDimension.x * volumeDimension.y * volumeDimension.z, data);

        glDeleteTextures(1, &computeSDFTextureId);

        if (m_writeToFile) {
            BinaryFileRW::writeBytes("../resources/volume/" + m_cellsFrameXXX + "-sdf-cityblock-approx.raw",
                                     2 * volumeDimension.x * volumeDimension.y * volumeDimension.z,
                                     data);
        }
    }

    GLuint sdfTextureId = 0;

    int mipLevels = static_cast<int>(glm::floor(glm::log2(
            static_cast<float>(glm::max(volumeDimension.x, glm::max(volumeDimension.y, volumeDimension.z))))));
    glGenTextures(1, &sdfTextureId);
    glBindTexture(GL_TEXTURE_3D, sdfTextureId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexStorage3D(GL_TEXTURE_3D, mipLevels + 1,
                   GL_R16F, volumeDimension.x, volumeDimension.y, volumeDimension.z);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, volumeDimension.x, volumeDimension.y, volumeDimension.z,
                    GL_RED,
                    GL_HALF_FLOAT, data);
    glBindTexture(GL_TEXTURE_3D, 0);

    MipMap mipMap;
    mipMap.init(GL_R16F, "mipmap_r16f");
    mipMap.execute(sdfTextureId, volumeDimension, mipLevels);
    mipMap.cleanUp();

    delete[] data;

    return sdfTextureId;
}

void SDF::cleanUp() {
    m_computeShaderProgram.cleanUp();
}

void SDF::reload() {
    cleanUp();
    init();
}
