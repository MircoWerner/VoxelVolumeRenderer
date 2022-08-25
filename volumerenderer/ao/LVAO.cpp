#include "LVAO.h"

void LVAO::init() {
    m_computeShaderProgram.init();
    m_computeShaderProgram.createComputeShader("../resources/shaders/volumerenderer/ao/lvao.glsl");
    m_computeShaderProgram.link();
    m_computeShaderProgram.createUniform("tex_positionBuffer");
    m_computeShaderProgram.createUniform("tex_normalBuffer");
    m_computeShaderProgram.createUniform("tex_voxelBuffer");
    m_computeShaderProgram.createUniform("tex_volume");
    ShaderProgram::unbind();
}

void LVAO::execute(GLuint aoBufferTextureId, GLuint positionBufferTextureId, GLuint normalBufferTextureId,
                   GLuint voxelBufferTextureId, GLuint volumeTextureId, int width, int height) {
    m_computeShaderProgram.bind();

    m_computeShaderProgram.setUniform("tex_positionBuffer", 0);
    m_computeShaderProgram.setUniform("tex_normalBuffer", 1);
    m_computeShaderProgram.setUniform("tex_voxelBuffer", 2);
    m_computeShaderProgram.setUniform("tex_volume", 3);

    glBindImageTexture(0, aoBufferTextureId, 0, false, 0, GL_READ_WRITE, GL_R32F);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionBufferTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalBufferTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, voxelBufferTextureId);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, volumeTextureId);

    m_computeShaderProgram.dispatch(width, height);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_R32F);

    ComputeShaderProgram::unbind();
}

void LVAO::cleanUp() {
    m_computeShaderProgram.cleanUp();
}

void LVAO::reload() {
    cleanUp();
    init();
}
