#include "Voxelization.h"

#include <iostream>
#include <chrono>

#include "../utils/MipMap.h"

void Voxelization::voxelization(GLuint volumeTextureId, glm::ivec3 volumeDimension, GLuint cellPropertiesBufferId) {
    ComputeShaderProgram computeShaderProgram;
    computeShaderProgram.init();
    computeShaderProgram.createComputeShader(
            "../resources/shaders/volumerenderer/utils/voxelization.glsl");
    computeShaderProgram.link();
    computeShaderProgram.createUniform("tex_volume");
    computeShaderProgram.createUniform("volumeDimension");
    computeShaderProgram.createUniform("compressedDimension");
    computeShaderProgram.createUniform("displacement");
    ShaderProgram::unbind();

    glm::ivec3 compressedDim = glm::ivec3(volumeDimension.x / 2 + volumeDimension.x % 2,
                                          volumeDimension.y / 2 + volumeDimension.y % 2,
                                          volumeDimension.z / 2 + volumeDimension.z % 2);

    int mipLevelsAlbedo = static_cast<int>(glm::floor(glm::log2(
            static_cast<float>(glm::max(compressedDim.x, glm::max(compressedDim.y, compressedDim.z))))));
    glGenTextures(1, &m_albedoTextureId);
    glBindTexture(GL_TEXTURE_3D, m_albedoTextureId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexStorage3D(GL_TEXTURE_3D, mipLevelsAlbedo + 1, GL_RGBA8, compressedDim.x, compressedDim.y, compressedDim.z);
    glBindTexture(GL_TEXTURE_3D, 0);

    int mipLevelsAlpha = static_cast<int>(glm::floor(glm::log2(
            static_cast<float>(glm::max(volumeDimension.x, glm::max(volumeDimension.y, volumeDimension.z))))));
    glGenTextures(1, &m_alphaTextureId);
    glBindTexture(GL_TEXTURE_3D, m_alphaTextureId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexStorage3D(GL_TEXTURE_3D, mipLevelsAlpha + 1, GL_R8, volumeDimension.x, volumeDimension.y, volumeDimension.z);
    glBindTexture(GL_TEXTURE_3D, 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, cellPropertiesBufferId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, volumeTextureId);

    glBindImageTexture(0, m_albedoTextureId, 0, false, 0, GL_READ_WRITE, GL_RGBA8);
    glBindImageTexture(1, m_alphaTextureId, 0, false, 0, GL_READ_WRITE, GL_R8);

    {
        computeShaderProgram.bind();
        computeShaderProgram.setUniform("tex_volume", 0);
        computeShaderProgram.setUniform("volumeDimension", volumeDimension);
        computeShaderProgram.setUniform("compressedDimension", compressedDim);

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        const int dimension = 64;
        for (int z = 0; z < volumeDimension.z; z += dimension) {
            for (int y = 0; y < volumeDimension.y; y += dimension) {
                for (int x = 0; x < volumeDimension.x; x += dimension) {
                    computeShaderProgram.setUniform("displacement", glm::ivec3(x, y, z));
                    computeShaderProgram.dispatch(dimension, dimension, dimension);
                }
            }
        }
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glFinish();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "[VOXELIZATION_Compute_Shader] Execution time = "
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                  << "[µs]" << std::endl;
    }

    glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA8);
    glBindImageTexture(1, 0, 0, false, 0, GL_READ_WRITE, GL_R8);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);

    glBindTexture(GL_TEXTURE_3D, 0);

    ComputeShaderProgram::unbind();

    computeShaderProgram.cleanUp();

    {
        MipMap mipMap;
        mipMap.init(GL_RGBA8, "mipmap_rgba8");
        mipMap.execute(m_albedoTextureId, compressedDim, mipLevelsAlbedo);
        mipMap.cleanUp();
    }
    {
        MipMap mipMap;
        mipMap.init(GL_R8, "mipmap_r8");
        mipMap.execute(m_alphaTextureId, volumeDimension, mipLevelsAlpha);
        mipMap.cleanUp();
    }
}

void Voxelization::cleanUp() {
    glDeleteTextures(1, &m_albedoTextureId);
    glDeleteTextures(1, &m_alphaTextureId);
}

void Voxelization::recalculate(GLuint volumeTextureId, glm::ivec3 volumeDimension, GLuint cellPropertiesBufferId) {
    glDeleteTextures(1, &m_albedoTextureId);
    glDeleteTextures(1, &m_alphaTextureId);
    voxelization(volumeTextureId, volumeDimension, cellPropertiesBufferId);
}
