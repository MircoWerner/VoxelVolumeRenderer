#include "RTAO.h"

void RTAO::init() {
    m_computeShaderProgram.init();
    m_computeShaderProgram.createComputeShader("../resources/shaders/volumerenderer/ao/rtao.glsl");
    m_computeShaderProgram.link();
    m_computeShaderProgram.createUniform("tex_positionBuffer");
    m_computeShaderProgram.createUniform("tex_normalBuffer");
    m_computeShaderProgram.createUniform("tex_volume");
    m_computeShaderProgram.createUniform("tex_sdf");
    m_computeShaderProgram.createUniform("volumeDimension");
    m_computeShaderProgram.createUniform("samples");
    m_computeShaderProgram.createUniform("distanceToHalfOcclusion");
    m_computeShaderProgram.createUniform("accumulatedSamples");
    ShaderProgram::unbind();
}

void RTAO::execute(GLuint aoBufferTextureId, GLuint positionBufferTextureId, GLuint normalBufferTextureId,
                   GLuint volumeTextureId, GLuint sdfTextureId, glm::ivec3 volumeDimension, int width,
                   int height, bool accumulationCall) {
    if (!accumulationCall) {
        m_accumulatedSamples = 0;
    }
    if (m_accumulatedSamples >= m_totalSamples) {
        return;
    }
    m_accumulatedSamples += m_samples;

    m_computeShaderProgram.bind();

    m_computeShaderProgram.setUniform("tex_positionBuffer", 0);
    m_computeShaderProgram.setUniform("tex_normalBuffer", 1);
    m_computeShaderProgram.setUniform("tex_volume", 2);
    m_computeShaderProgram.setUniform("tex_sdf", 3);
    m_computeShaderProgram.setUniform("volumeDimension", volumeDimension);
    m_computeShaderProgram.setUniform("samples", m_samples);
    m_computeShaderProgram.setUniform("distanceToHalfOcclusion", m_distanceToHalfOcclusion);
    m_computeShaderProgram.setUniform("accumulatedSamples", m_accumulatedSamples);

    glBindImageTexture(0, aoBufferTextureId, 0, false, 0, GL_READ_WRITE, GL_R32F);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionBufferTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalBufferTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, volumeTextureId);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, sdfTextureId);

    m_computeShaderProgram.dispatch(width, height);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_R32F);

    ComputeShaderProgram::unbind();
}

void RTAO::cleanUp() {
    m_computeShaderProgram.cleanUp();
}

void RTAO::reload() {
    cleanUp();
    init();
}
