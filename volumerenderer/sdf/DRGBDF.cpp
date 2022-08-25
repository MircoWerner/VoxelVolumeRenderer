#include "DRGBDF.h"

#include <chrono>
#include <iostream>

#include "../utils/BinaryFileRW.h"

DRGBDF::DRGBDF(bool readFromFile, bool writeToFile, std::string cellsFrameXXX) :
        m_readFromFile(readFromFile), m_writeToFile(writeToFile), m_cellsFrameXXX(std::move(cellsFrameXXX)) {

}

void DRGBDF::init() {
    m_computeShaderProgramInitialize.init();
    m_computeShaderProgramInitialize.createComputeShader(
            "../resources/shaders/volumerenderer/df/rgb/drgbdf_initializer.glsl");
    m_computeShaderProgramInitialize.link();
    m_computeShaderProgramInitialize.createUniform("tex_volume");
    m_computeShaderProgramInitialize.createUniform("rgbDimension");
    m_computeShaderProgramInitialize.createUniform("displacement");
    m_computeShaderProgramInitialize.createUniform("rgbIdx");
    m_computeShaderProgramInitialize.createUniform("zDisplacement");
    ShaderProgram::unbind();
    m_computeShaderProgramFilter.init();
    m_computeShaderProgramFilter.createComputeShader(
            "../resources/shaders/volumerenderer/df/rgb/drgbdf_rgba32ui.glsl");
    m_computeShaderProgramFilter.link();
    m_computeShaderProgramFilter.createUniform("tex_volume");
    m_computeShaderProgramFilter.createUniform("rgbDimension");
    m_computeShaderProgramFilter.createUniform("displacement");
    m_computeShaderProgramFilter.createUniform("zDisplacement");
    ShaderProgram::unbind();
    m_computeShaderProgramConvert.init();
    m_computeShaderProgramConvert.createComputeShader(
            "../resources/shaders/volumerenderer/df/rgb/drgbdf_converter.glsl");
    m_computeShaderProgramConvert.link();
    m_computeShaderProgramConvert.createUniform("tex_directional_rgb_df");
    m_computeShaderProgramConvert.createUniform("rgbDimension");
    m_computeShaderProgramConvert.createUniform("displacement");
    m_computeShaderProgramConvert.createUniform("rgbIdx");
    m_computeShaderProgramConvert.createUniform("zDisplacement");
    ShaderProgram::unbind();
}

GLuint DRGBDF::execute(GLuint volumeTextureId, glm::ivec3 volumeDimension, GLuint cellPropertiesBufferId) {
    GLuint rgbdfTextureId = 0;
    GLuint directionalTextureId = 0;

    glGenTextures(1, &rgbdfTextureId);
    glBindTexture(GL_TEXTURE_3D, rgbdfTextureId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R16I, volumeDimension.x, volumeDimension.y, volumeDimension.z);
    glBindTexture(GL_TEXTURE_3D, 0);

    const int zIncrease = 300;
    const glm::ivec3 rgbDimension = glm::ivec3(volumeDimension.x, volumeDimension.y, zIncrease);

    glGenTextures(1, &directionalTextureId);
    glBindTexture(GL_TEXTURE_3D, directionalTextureId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32UI, rgbDimension.x, rgbDimension.y, rgbDimension.z);
    glBindTexture(GL_TEXTURE_3D, 0);

    for (int zDisplacement = 0; zDisplacement < volumeDimension.z; zDisplacement += (zIncrease - 16)) {
        std::cout << "[DIRECTIONAL_RGB_DF] " << zDisplacement << ".." << (zDisplacement + zIncrease) << std::endl;
        for (int rgbIdx = 0; rgbIdx < 3; rgbIdx++) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_3D, volumeTextureId);
            glBindImageTexture(0, directionalTextureId, 0, false, 0, GL_READ_WRITE, GL_RGBA32UI);

            {
                // begin INITIALIZE compute shader
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellPropertiesBufferId);

                m_computeShaderProgramInitialize.bind();
                m_computeShaderProgramInitialize.setUniform("tex_volume", 0);
                m_computeShaderProgramInitialize.setUniform("rgbDimension", rgbDimension);
                m_computeShaderProgramInitialize.setUniform("rgbIdx", rgbIdx);
                m_computeShaderProgramInitialize.setUniform("zDisplacement", zDisplacement);

                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
                const int dimension = 64;
                for (int z = 0; z < rgbDimension.z; z += dimension) {
                    for (int y = 0; y < rgbDimension.y; y += dimension) {
                        for (int x = 0; x < rgbDimension.x; x += dimension) {
                            m_computeShaderProgramInitialize.setUniform("displacement", glm::ivec3(x, y, z));
                            m_computeShaderProgramInitialize.dispatch(dimension, dimension, dimension);
                        }
                    }
                }
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                glFinish();
                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                std::cout << "[DIRECTIONAL_RGB_DF_INITIALIZE_Compute_Shader] Execution time = "
                          << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                          << "[µs]" << std::endl;

                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
                // end INITIALIZE compute shader
            }
            {
                // begin FILTER compute shader
                m_computeShaderProgramFilter.bind();
                m_computeShaderProgramFilter.setUniform("tex_volume", 0);
                m_computeShaderProgramFilter.setUniform("rgbDimension", rgbDimension);
                m_computeShaderProgramFilter.setUniform("zDisplacement", zDisplacement);

                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
                const int dimension = 64;
                for (int i = 0; i < 16; i++) {
                    for (int z = 0; z < rgbDimension.z; z += dimension) {
                        for (int y = 0; y < rgbDimension.y; y += dimension) {
                            for (int x = 0; x < rgbDimension.x; x += dimension) {
                                m_computeShaderProgramFilter.setUniform("displacement", glm::ivec3(x, y, z));
                                m_computeShaderProgramFilter.dispatch(dimension, dimension, dimension);
                            }
                        }
                    }
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                    glFinish();
                }
                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                std::cout << "[DIRECTIONAL_RGB_DF_FILTER_Compute_Shader] Execution time = "
                          << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                          << "[µs]" << std::endl;
                // end FILTER compute shader
            }
            glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32UI);
            glBindTexture(GL_TEXTURE_3D, 0);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_3D, directionalTextureId);
            glBindImageTexture(0, rgbdfTextureId, 0, false, 0, GL_READ_WRITE, GL_R16UI);
            {
                // begin CONVERT compute shader
                m_computeShaderProgramConvert.bind();
                m_computeShaderProgramConvert.setUniform("tex_directional_rgb_df", 0);
                m_computeShaderProgramConvert.setUniform("rgbDimension", rgbDimension);
                m_computeShaderProgramConvert.setUniform("rgbIdx", rgbIdx);
                m_computeShaderProgramConvert.setUniform("zDisplacement", zDisplacement);

                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
                const int dimension = 64;
                for (int z = 0; z < rgbDimension.z; z += dimension) {
                    for (int y = 0; y < rgbDimension.y; y += dimension) {
                        for (int x = 0; x < rgbDimension.x; x += dimension) {
                            m_computeShaderProgramConvert.setUniform("displacement", glm::ivec3(x, y, z));
                            m_computeShaderProgramConvert.dispatch(dimension, dimension, dimension);
                        }
                    }
                }
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                glFinish();
                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                std::cout << "[DIRECTIONAL_RGB_DF_CONVERT_Compute_Shader] Execution time = "
                          << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                          << "[µs]" << std::endl;
                // end CONVERT compute shader
            }
            ComputeShaderProgram::unbind();
        }
    }
    glDeleteTextures(1, &directionalTextureId);
    return rgbdfTextureId;
}

void DRGBDF::cleanUp() {
    m_computeShaderProgramInitialize.cleanUp();
    m_computeShaderProgramFilter.cleanUp();
    m_computeShaderProgramConvert.cleanUp();
}

GLuint DRGBDF::load(GLuint volumeTextureId, glm::ivec3 volumeDimension, GLuint cellPropertiesBufferId) {
    GLuint rgbdfTextureId = 0;

    if (m_readFromFile &&
        BinaryFileRW::exists("../resources/volume/" + m_cellsFrameXXX + "-dir_rgb_df.raw")) {
        char *data = new char[2 * volumeDimension.x * volumeDimension.y * volumeDimension.z];
        BinaryFileRW::readBytes("../resources/volume/" + m_cellsFrameXXX + "-dir_rgb_df.raw",
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
            BinaryFileRW::writeBytes("../resources/volume/" + m_cellsFrameXXX + "-dir_rgb_df.raw",
                                     2 * volumeDimension.x * volumeDimension.y * volumeDimension.z,
                                     data);
        }
    }

    return rgbdfTextureId;
}
