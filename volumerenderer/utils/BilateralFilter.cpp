#include "BilateralFilter.h"

void BilateralFilter::init() {
    m_computeShaderProgram.init();
    m_computeShaderProgram.createComputeShader("../resources/shaders/volumerenderer/utils/bilateral_filter.glsl");
    m_computeShaderProgram.link();
    m_computeShaderProgram.createUniform("tex_voxelBuffer");
    m_computeShaderProgram.createUniform("tex_normalBuffer");
    m_computeShaderProgram.createUniform("tex_positionBuffer");
    m_computeShaderProgram.createUniform("tex_ambientOcclusionBuffer");
    m_computeShaderProgram.createUniform("kernelRadius");
    m_computeShaderProgram.createUniform("sigmaSpatial");
    ShaderProgram::unbind();
}

void BilateralFilter::execute(GLuint ambientOcclusionBufferTextureId, GLuint voxelBufferTextureId,
                              GLuint normalBufferTextureId, GLuint positionBufferTextureId, int width, int height,
                              GLuint m_ambientOcclusionBufferFilterTargetTextureId) {
    m_computeShaderProgram.bind();

    m_computeShaderProgram.setUniform("tex_voxelBuffer", 0);
    m_computeShaderProgram.setUniform("tex_normalBuffer", 1);
    m_computeShaderProgram.setUniform("tex_positionBuffer", 2);
    m_computeShaderProgram.setUniform("tex_ambientOcclusionBuffer", 3);
    m_computeShaderProgram.setUniform("kernelRadius", m_kernelRadius);
    m_computeShaderProgram.setUniform("sigmaSpatial", m_sigmaSpatial);

    glBindImageTexture(0, m_ambientOcclusionBufferFilterTargetTextureId, 0, false, 0, GL_WRITE_ONLY, GL_R32F);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, voxelBufferTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalBufferTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, positionBufferTextureId);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ambientOcclusionBufferTextureId);

    m_computeShaderProgram.dispatch(width, height);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_R32F);

    ComputeShaderProgram::unbind();
}

void BilateralFilter::cleanUp() {
    m_computeShaderProgram.cleanUp();
}

void BilateralFilter::reload() {
    cleanUp();
    init();
}
