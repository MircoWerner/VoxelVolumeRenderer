#include "VDCAO.h"

void VDCAO::init() {
    m_computeShaderProgram.init();
    m_computeShaderProgram.createComputeShader("../resources/shaders/volumerenderer/ao/vdcao.glsl");
    m_computeShaderProgram.link();
    m_computeShaderProgram.createUniform("tex_positionBuffer");
    m_computeShaderProgram.createUniform("tex_normalBuffer");
    m_computeShaderProgram.createUniform("tex_voxelBuffer");
    m_computeShaderProgram.createUniform("tex_volume");
    m_computeShaderProgram.createUniform("tex_sdf");
    m_computeShaderProgram.createUniform("volumeDimension");

    m_computeShaderProgram.createUniform("coneTraceSteps");
    m_computeShaderProgram.createUniform("coneTraceStepSize");
    m_computeShaderProgram.createUniform("coneApertureAngle");
    m_computeShaderProgram.createUniform("attenuation");
    ShaderProgram::unbind();
}

void
VDCAO::execute(GLuint aoBufferTextureId, GLuint positionBufferTextureId,
               GLuint normalBufferTextureId, GLuint voxelBufferTextureId, GLuint volumeTextureId,
               GLuint sdfTextureId, glm::ivec3 volumeDimension, int width, int height) {
    m_computeShaderProgram.bind();

    m_computeShaderProgram.setUniform("tex_positionBuffer", 0);
    m_computeShaderProgram.setUniform("tex_normalBuffer", 1);
    m_computeShaderProgram.setUniform("tex_voxelBuffer", 2);
    m_computeShaderProgram.setUniform("tex_volume", 3);
    m_computeShaderProgram.setUniform("tex_sdf", 4);
    m_computeShaderProgram.setUniform("volumeDimension", volumeDimension);

    m_computeShaderProgram.setUniform("coneTraceSteps", m_coneTraceSteps);
    m_computeShaderProgram.setUniform("coneTraceStepSize", m_coneTraceStepSize);
    m_computeShaderProgram.setUniform("coneApertureAngle", glm::radians(m_coneApertureAngle));
    m_computeShaderProgram.setUniform("attenuation", m_attenuation);

    glBindImageTexture(0, aoBufferTextureId, 0, false, 0, GL_READ_WRITE, GL_R32F);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionBufferTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalBufferTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, voxelBufferTextureId);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, volumeTextureId);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_3D, sdfTextureId);

    m_computeShaderProgram.dispatch(width, height);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_R32F);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);

    ComputeShaderProgram::unbind();
}

void VDCAO::cleanUp() {
    m_computeShaderProgram.cleanUp();
}

void VDCAO::reload() {
    cleanUp();
    init();
}
