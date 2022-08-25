#include "HBAO.h"

void HBAO::init() {
    m_computeShaderProgram.init();
    m_computeShaderProgram.createComputeShader("../resources/shaders/volumerenderer/ao/hbao.glsl");
    m_computeShaderProgram.link();
    m_computeShaderProgram.createUniform("tex_depthBuffer");
    m_computeShaderProgram.createUniform("tex_normalBuffer");
    m_computeShaderProgram.createUniform("tex_positionBuffer");
    m_computeShaderProgram.createUniform("viewMatrix");
    m_computeShaderProgram.createUniform("inverseProjection");
    m_computeShaderProgram.createUniform("Nd");
    m_computeShaderProgram.createUniform("Ns");
    m_computeShaderProgram.createUniform("stepSize");
    m_computeShaderProgram.createUniform("R");
    m_computeShaderProgram.createUniform("tangentBias");
    ShaderProgram::unbind();
}

void HBAO::execute(GLuint ambientOcclusionBufferTextureId, GLuint depthBufferTextureId, GLuint normalBufferTextureId,
                   GLuint positionBufferTextureId, int width, int height, glm::mat4 MV, glm::mat4 inverseProjection) {
    m_computeShaderProgram.bind();

    m_computeShaderProgram.setUniform("tex_depthBuffer", 0);
    m_computeShaderProgram.setUniform("tex_normalBuffer", 1);
    m_computeShaderProgram.setUniform("tex_positionBuffer", 2);
    m_computeShaderProgram.setUniform("viewMatrix", MV);
    m_computeShaderProgram.setUniform("inverseProjection", inverseProjection);
    m_computeShaderProgram.setUniform("Nd", m_Nd);
    m_computeShaderProgram.setUniform("Ns", m_Ns);
    m_computeShaderProgram.setUniform("stepSize", m_stepSize);
    m_computeShaderProgram.setUniform("R", m_R);
    m_computeShaderProgram.setUniform("tangentBias", m_tangentBias);

    glBindImageTexture(0, ambientOcclusionBufferTextureId, 0, false, 0, GL_READ_WRITE, GL_R32F);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthBufferTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalBufferTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, positionBufferTextureId);

    m_computeShaderProgram.dispatch(width, height);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_R32F);

    ComputeShaderProgram::unbind();
}

void HBAO::cleanUp() {
    m_computeShaderProgram.cleanUp();
}

void HBAO::reload() {
    cleanUp();
    init();
}
