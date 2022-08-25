#include "RGBDF.h"

#include <chrono>
#include <iostream>

#include "../utils/BinaryFileRW.h"

void RGBDF::init() {
    m_computeShaderProgramInitialize.init();
    m_computeShaderProgramInitialize.createComputeShader(
            "../resources/shaders/volumerenderer/df/rgb/rgbdf_initializer.glsl");
    m_computeShaderProgramInitialize.link();
    m_computeShaderProgramInitialize.createUniform("tex_volume");
    m_computeShaderProgramInitialize.createUniform("volumeDimension");
    m_computeShaderProgramInitialize.createUniform("displacement");
    ShaderProgram::unbind();
    m_computeShaderProgramFilter.init();
    m_computeShaderProgramFilter.createComputeShader(
            "../resources/shaders/volumerenderer/df/rgb/rgbdf_r16i.glsl");
    m_computeShaderProgramFilter.link();
    m_computeShaderProgramFilter.createUniform("tex_volume");
    m_computeShaderProgramFilter.createUniform("volumeDimension");
    m_computeShaderProgramFilter.createUniform("displacement");
    ShaderProgram::unbind();
}

GLuint RGBDF::execute(GLuint volumeTextureId, glm::ivec3 volumeDimension, GLuint cellPropertiesBufferId) {
    GLuint rgbsdfTextureId = 0;

    glGenTextures(1, &rgbsdfTextureId);
    glBindTexture(GL_TEXTURE_3D, rgbsdfTextureId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R16I, volumeDimension.x, volumeDimension.y, volumeDimension.z);
    glBindTexture(GL_TEXTURE_3D, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, volumeTextureId);

    {
        // begin RGB_SDF_INITIALIZE compute shader
        glBindImageTexture(0, rgbsdfTextureId, 0, false, 0, GL_WRITE_ONLY, GL_R16UI);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellPropertiesBufferId);

        m_computeShaderProgramInitialize.bind();
        m_computeShaderProgramInitialize.setUniform("tex_volume", 0);
        m_computeShaderProgramInitialize.setUniform("volumeDimension", volumeDimension);

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        const int dimension = 64;
        for (int z = 0; z < volumeDimension.z; z += dimension) {
            for (int y = 0; y < volumeDimension.y; y += dimension) {
                for (int x = 0; x < volumeDimension.x; x += dimension) {
                    m_computeShaderProgramInitialize.setUniform("displacement", glm::ivec3(x, y, z));
                    m_computeShaderProgramInitialize.dispatch(dimension, dimension, dimension);
                }
            }
        }
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glFinish();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "[RGB_DF_Initialize_Compute_Shader] Execution time = "
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                  << "[µs]" << std::endl;

        glBindImageTexture(0, 0, 0, false, 0, GL_WRITE_ONLY, GL_R16UI);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
        // end RGB_SDF_INITIALIZE compute shader
    }

    {
        // begin RGB_SDF_INITIALIZE compute shader
        glBindImageTexture(0, rgbsdfTextureId, 0, false, 0, GL_READ_WRITE, GL_R16UI);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellPropertiesBufferId);

        m_computeShaderProgramFilter.bind();
        m_computeShaderProgramFilter.setUniform("tex_volume", 0);
        m_computeShaderProgramFilter.setUniform("volumeDimension", volumeDimension);

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        const int dimension = 64;
        for (int i = 0; i < 16; i++) {
            for (int z = 0; z < volumeDimension.z; z += dimension) {
                for (int y = 0; y < volumeDimension.y; y += dimension) {
                    for (int x = 0; x < volumeDimension.x; x += dimension) {
                        m_computeShaderProgramFilter.setUniform("displacement", glm::ivec3(x, y, z));
                        m_computeShaderProgramFilter.dispatch(dimension, dimension, dimension);
                    }
                }
            }
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glFinish();
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "[RGB_DF_Filter_Compute_Shader] Execution time = "
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                  << "[µs]" << std::endl;

        glBindImageTexture(0, 0, 0, false, 0, GL_WRITE_ONLY, GL_R16UI);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
        // end RGB_SDF_INITIALIZE compute shader
    }

    glBindTexture(GL_TEXTURE_3D, 0);

    ComputeShaderProgram::unbind();

    return rgbsdfTextureId;
}

void RGBDF::cleanUp() {
    m_computeShaderProgramInitialize.cleanUp();
    m_computeShaderProgramFilter.cleanUp();
}

RGBDF::RGBDF(bool readFromFile, bool writeToFile, std::string cellsFrameXXX) :
        m_readFromFile(readFromFile), m_writeToFile(writeToFile), m_cellsFrameXXX(std::move(cellsFrameXXX)) {

}

GLuint RGBDF::load(GLuint volumeTextureId, glm::ivec3 volumeDimension, GLuint cellPropertiesBufferId) {
    GLuint rgbdfTextureId = 0;

    if (m_readFromFile &&
        BinaryFileRW::exists("../resources/volume/" + m_cellsFrameXXX + "-rgb_df.raw")) {
        char *data = new char[2 * volumeDimension.x * volumeDimension.y * volumeDimension.z];
        BinaryFileRW::readBytes("../resources/volume/" + m_cellsFrameXXX + "-rgb_df.raw",
                                2 * volumeDimension.x * volumeDimension.y * volumeDimension.z,
                                data);

        glGenTextures(1, &rgbdfTextureId);
        glBindTexture(GL_TEXTURE_3D, rgbdfTextureId);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R16I, volumeDimension.x, volumeDimension.y, volumeDimension.z, 0,
                     GL_RED_INTEGER, GL_SHORT, data);
        glBindTexture(GL_TEXTURE_3D, 0);

        delete[] data;
    } else {
        init();
        rgbdfTextureId = execute(volumeTextureId, volumeDimension, cellPropertiesBufferId);
        cleanUp();

        if (m_writeToFile) {
            char *data = new char[2 * volumeDimension.x * volumeDimension.y * volumeDimension.z];
            glGetTextureImage(rgbdfTextureId, 0, GL_RED_INTEGER, GL_SHORT,
                              2 * volumeDimension.x * volumeDimension.y * volumeDimension.z, data);
            BinaryFileRW::writeBytes("../resources/volume/" + m_cellsFrameXXX + "-rgb_df.raw",
                                     2 * volumeDimension.x * volumeDimension.y * volumeDimension.z,
                                     data);
        }
    }

    return rgbdfTextureId;
}
