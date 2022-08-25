#include "VCTE.h"

#include <iostream>
#include <chrono>

void VCTE::init() {
    m_computeShaderProgram.init();
    m_computeShaderProgram.createComputeShader(
            "../resources/shaders/volumerenderer/emission/vcte.glsl");
    m_computeShaderProgram.link();
    m_computeShaderProgram.createUniform("tex_volume");
    m_computeShaderProgram.createUniform("tex_positionBuffer");
    m_computeShaderProgram.createUniform("tex_normalBuffer");
    m_computeShaderProgram.createUniform("tex_voxelBuffer");
    m_computeShaderProgram.createUniform("volumeDimension");
    m_computeShaderProgram.createUniform("rayOrigin");
    m_computeShaderProgram.createUniform("tex_albedo");
    m_computeShaderProgram.createUniform("tex_alpha");

    m_computeShaderProgram.createUniform("calcDiffuse");
    m_computeShaderProgram.createUniform("diffuseConeTraceSteps");
    m_computeShaderProgram.createUniform("diffuseConeTraceStepSize");
    m_computeShaderProgram.createUniform("diffuseConeApertureAngle");

    m_computeShaderProgram.createUniform("calcSpecular");
    m_computeShaderProgram.createUniform("specularConeTraceSteps");
    m_computeShaderProgram.createUniform("specularConeTraceStepSize");
    m_computeShaderProgram.createUniform("specularConeApertureAngle");
    ShaderProgram::unbind();
}

void VCTE::execute(GLuint emissionBufferTextureId, GLuint positionBufferTextureId, GLuint normalBufferTextureId,
                   GLuint voxelBufferTextureId, GLuint albedoTextureId,
                   GLuint alphaTextureId, GLuint volumeTextureId, glm::ivec3 volumeDimension, int width, int height,
                   glm::vec3 rayOrigin, GLuint cellPropertiesBufferId) {
    m_computeShaderProgram.bind();

    m_computeShaderProgram.setUniform("tex_volume", 0);
    m_computeShaderProgram.setUniform("tex_positionBuffer", 1);
    m_computeShaderProgram.setUniform("tex_normalBuffer", 2);
    m_computeShaderProgram.setUniform("tex_voxelBuffer", 3);
    m_computeShaderProgram.setUniform("volumeDimension", volumeDimension);
    m_computeShaderProgram.setUniform("rayOrigin", rayOrigin);
    m_computeShaderProgram.setUniform("tex_albedo", 4);
    m_computeShaderProgram.setUniform("tex_alpha", 5);

    m_computeShaderProgram.setUniform("calcDiffuse", m_diffuse);
    m_computeShaderProgram.setUniform("diffuseConeTraceSteps", m_diffuseConeTraceSteps);
    m_computeShaderProgram.setUniform("diffuseConeTraceStepSize", m_diffuseConeTraceStepSize);
    m_computeShaderProgram.setUniform("diffuseConeApertureAngle", glm::radians(m_diffuseConeApertureAngle));

    m_computeShaderProgram.setUniform("calcSpecular", m_specular);
    m_computeShaderProgram.setUniform("specularConeTraceSteps", m_specularConeTraceSteps);
    m_computeShaderProgram.setUniform("specularConeTraceStepSize", m_specularConeTraceStepSize);
    m_computeShaderProgram.setUniform("specularConeApertureAngle", glm::radians(m_specularConeApertureAngle));

    glBindImageTexture(0, emissionBufferTextureId, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellPropertiesBufferId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, volumeTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, positionBufferTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normalBufferTextureId);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, voxelBufferTextureId);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_3D, albedoTextureId);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_3D, alphaTextureId);

    m_computeShaderProgram.dispatch(width, height);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_3D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

    ComputeShaderProgram::unbind();
}

void VCTE::cleanUp() {
    m_computeShaderProgram.cleanUp();
}

void VCTE::reload() {
    cleanUp();
    init();
}
