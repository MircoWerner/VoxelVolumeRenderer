#include "VCTAO.h"

void VCTAO::init() {
    m_computeShaderProgram.init();
    m_computeShaderProgram.createComputeShader(
            "../resources/shaders/volumerenderer/ao/vctao.glsl");
    m_computeShaderProgram.link();
    m_computeShaderProgram.createUniform("tex_positionBuffer");
    m_computeShaderProgram.createUniform("tex_normalBuffer");
    m_computeShaderProgram.createUniform("volumeDimension");
    m_computeShaderProgram.createUniform("tex_alpha");

    m_computeShaderProgram.createUniform("coneTraceSteps");
    m_computeShaderProgram.createUniform("coneTraceStepSize");
    m_computeShaderProgram.createUniform("coneApertureAngle");
    m_computeShaderProgram.createUniform("attenuation");
    ShaderProgram::unbind();
}

void VCTAO::execute(GLuint aoBufferTextureId, GLuint positionBufferTextureId, GLuint normalBufferTextureId,
                    GLuint alphaTextureId, glm::ivec3 volumeDimension, int width, int height) {
    m_computeShaderProgram.bind();

    m_computeShaderProgram.setUniform("tex_positionBuffer", 0);
    m_computeShaderProgram.setUniform("tex_normalBuffer", 1);
    m_computeShaderProgram.setUniform("volumeDimension", volumeDimension);
    m_computeShaderProgram.setUniform("tex_alpha", 2);

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
    glBindTexture(GL_TEXTURE_3D, alphaTextureId);

    m_computeShaderProgram.dispatch(width, height);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_3D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_R32F);
    glBindImageTexture(1, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);

    ComputeShaderProgram::unbind();
}

void VCTAO::cleanUp() {
    m_computeShaderProgram.cleanUp();
}

void VCTAO::reload() {
    cleanUp();
    init();
}
