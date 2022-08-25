#include "VolumeRenderer.h"

#include <random>

#include "../../renderengine/utils/Transformation.h"

void VolumeRenderer::init(int windowWidth, int windowHeight) {
    // shader programs
    {
        reloadVertexFragmentShader();
        reloadVolumeComputeShader();

        // vertex data, full screen quad
        std::vector<float> vertices = {
                -1.f, -1.f, 0.f, // lower left
                1.f, -1.f, 0.f, // lower right
                1.f, 1.f, 0.f, // top right
                -1.f, 1.f, 0.f, // top left
        };
        std::vector<float> textures = {
                0.f, 0.f,
                1.f, 0.f,
                1.f, 1.f,
                0.f, 1.f,
        };
        std::vector<int> indices = {
                0, 2, 3,
                0, 1, 2,
        };
        count = indices.size();

        glGenVertexArrays(1, &vaoId);
        glBindVertexArray(vaoId);

        glGenBuffers(1, &vertexVboId);
        glBindBuffer(GL_ARRAY_BUFFER, vertexVboId);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);

        glGenBuffers(1, &textureVboId);
        glBindBuffer(GL_ARRAY_BUFFER, textureVboId);
        glBufferData(GL_ARRAY_BUFFER, textures.size() * sizeof(float), textures.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, nullptr);

        glGenBuffers(1, &indicesVboId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesVboId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // volume
    {
        m_ambientOcclusion.init();
        m_emission.init();
        m_bilateralFilter.init();

        resizeFramebuffers(windowWidth, windowHeight);

        m_volume.load();
    }
}


void VolumeRenderer::render(ACamera *camera, int windowWidth, int windowHeight, bool executeComputeShader) {
    static const float FOV = glm::radians(90.0f);
    static const float Z_NEAR = 0.01f;
    static const float Z_FAR = 1000.f;

    if (executeComputeShader) {
        computeShaderProgram.bind();

        glm::mat4 projectionMatrix = Transformation::getProjectionMatrix(FOV, static_cast<float>(windowWidth),
                                                                         static_cast<float>(windowHeight), Z_NEAR,
                                                                         Z_FAR);
        glm::mat4 viewMatrix = Transformation::getViewMatrix(camera);
        glm::mat4 inverseProjection = glm::inverse(projectionMatrix);
        glm::mat4 inverseView = glm::inverse(viewMatrix);

        computeShaderProgram.setUniform("ray00", ndcToWorldSpace(-1.f, -1.f, inverseProjection, inverseView));
        computeShaderProgram.setUniform("ray01", ndcToWorldSpace(-1.f, 1.f, inverseProjection, inverseView));
        computeShaderProgram.setUniform("ray10", ndcToWorldSpace(1.f, -1.f, inverseProjection, inverseView));
        computeShaderProgram.setUniform("ray11", ndcToWorldSpace(1.f, 1.f, inverseProjection, inverseView));
        computeShaderProgram.setUniform("rayOrigin", camera->getPosition());

        computeShaderProgram.setUniform("tex_volume", 0);
        computeShaderProgram.setUniform("tex_sdf", 1);
        computeShaderProgram.setUniform("volumeDimension", m_volume.m_volumeDimension);

        computeShaderProgram.setUniform("udbgSdf", m_debugSdf ? 1 : 0);
        computeShaderProgram.setUniform("udbgSdfValue", m_debugSdfValue);

        computeShaderProgram.setUniform("MVP", projectionMatrix * viewMatrix);

        glBindImageTexture(0, m_positionBufferTextureId, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
        glBindImageTexture(1, m_colorBufferTextureId, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
        glBindImageTexture(2, m_depthBufferTextureId, 0, false, 0, GL_READ_WRITE, GL_R32F);
        glBindImageTexture(3, m_normalBufferTextureId, 0, false, 0, GL_READ_WRITE, GL_R8UI);
        glBindImageTexture(4, m_voxelBufferTextureId, 0, false, 0, GL_READ_WRITE, GL_RGBA16I);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_volume.m_cellPropertiesBufferId);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, m_volume.m_volumeTextureId);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, m_volume.m_sdfTextureId);

        computeShaderProgram.dispatch(windowWidth, windowHeight);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glBindTexture(GL_TEXTURE_3D, 0);

        glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
        glBindImageTexture(1, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
        glBindImageTexture(2, 0, 0, false, 0, GL_READ_WRITE, GL_R32F);
        glBindImageTexture(3, 0, 0, false, 0, GL_READ_WRITE, GL_R8UI);
        glBindImageTexture(4, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA16I);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0);

        ComputeShaderProgram::unbind();

        m_ambientOcclusion.execute(m_ambientOcclusionBufferTextureId, m_positionBufferTextureId,
                                   m_normalBufferTextureId, m_voxelBufferTextureId, m_depthBufferTextureId,
                                   m_volume.m_volumeTextureId, m_volume.m_sdfTextureId,
                                   m_volume.m_voxelization.m_alphaTextureId,
                                   m_volume.m_volumeDimension, windowWidth, windowHeight, viewMatrix,
                                   inverseProjection);
        if (m_ambientOcclusion.m_method == AmbientOcclusion::AO::AO_HBAO &&
            m_ambientOcclusion.m_hbao.m_executeBilateralFilter) {
            executeBilateralFilter(windowWidth, windowHeight);
        }
        m_emission.execute(m_emissionBufferTextureId, m_positionBufferTextureId,
                           m_normalBufferTextureId, m_voxelBufferTextureId,
                           m_volume.m_voxelization.m_albedoTextureId,
                           m_volume.m_voxelization.m_alphaTextureId,
                           m_volume.m_rgbdfTextureId,
                           m_volume.m_volumeTextureId, m_volume.m_volumeDimension, windowWidth, windowHeight,
                           camera->getPosition(), m_volume.m_cellPropertiesBufferId);
    }

    if (!executeComputeShader) {
        m_ambientOcclusion.executeAccumulationCall(m_ambientOcclusionBufferTextureId, m_positionBufferTextureId,
                                                   m_normalBufferTextureId, m_volume.m_volumeTextureId,
                                                   m_volume.m_sdfTextureId,
                                                   m_volume.m_volumeDimension, windowWidth, windowHeight);
    }

    shaderProgram.bind();

    shaderProgram.setUniform("tex_albedoBuffer", 0);
    shaderProgram.setUniform("tex_ambientOcclusionBuffer", 1);
    shaderProgram.setUniform("tex_emissionBuffer", 2);
    shaderProgram.setUniform("tex_normalBuffer", 3);
    shaderProgram.setUniform("tex_depthBuffer", 4);
    shaderProgram.setUniform("ambientOcclusionEnabled", m_ambientOcclusion.m_method > 0);
    shaderProgram.setUniform("emissionEnabled", m_emission.m_method > 0);
    shaderProgram.setUniform("emissionFactor", m_emissionFactor);
    shaderProgram.setUniform("dbgMode", m_fragmentDebugMode);
    shaderProgram.setUniform("nearPlane", Z_NEAR);
    shaderProgram.setUniform("farPlane", Z_FAR);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_colorBufferTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_ambientOcclusionBufferTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_emissionBufferTextureId);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_normalBufferTextureId);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_depthBufferTextureId);

    glBindVertexArray(vaoId);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDepthMask(GL_FALSE);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    glDepthMask(GL_TRUE);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);

    ShaderProgram::unbind();
}

void VolumeRenderer::cleanUp() {
    glDeleteBuffers(1, &vertexVboId);
    glDeleteBuffers(1, &textureVboId);
    glDeleteBuffers(1, &indicesVboId);
    glDeleteVertexArrays(1, &vaoId);
    cleanUpFramebuffers();
    m_volume.cleanUp();
    computeShaderProgram.cleanUp();
    shaderProgram.cleanUp();
    m_ambientOcclusion.cleanUp();
    m_bilateralFilter.cleanUp();
}

glm::vec3 VolumeRenderer::ndcToWorldSpace(float ndcX, float ndcY, glm::mat4 inverseProjection, glm::mat4 inverseView) {
    glm::vec4 clipCoordinates = glm::vec4(ndcX, ndcY, -1.f, 1.f);
    glm::vec4 viewCoordinates = inverseProjection * clipCoordinates;
    viewCoordinates.z = -1.f;
    viewCoordinates.w = 0.f;
    glm::vec4 worldCoordinates = inverseView * viewCoordinates;
    return glm::normalize(glm::vec3(worldCoordinates.x, worldCoordinates.y, worldCoordinates.z));
}

void VolumeRenderer::cleanUpFramebuffers() {
    glDeleteTextures(1, &m_positionBufferTextureId);
    glDeleteTextures(1, &m_colorBufferTextureId);
    glDeleteTextures(1, &m_ambientOcclusionBufferTextureId);
    glDeleteTextures(1, &m_ambientOcclusionBufferFilterTargetTextureId);
    glDeleteTextures(1, &m_depthBufferTextureId);
    glDeleteTextures(1, &m_normalBufferTextureId);
    glDeleteTextures(1, &m_voxelBufferTextureId);
    glDeleteTextures(1, &m_emissionBufferTextureId);
}

void VolumeRenderer::resizeFramebuffers(int windowWidth, int windowHeight) {
    cleanUpFramebuffers();
    m_positionBufferTextureId = createImageBuffer(windowWidth, windowHeight, GL_RGBA32F);
    m_colorBufferTextureId = createImageBuffer(windowWidth, windowHeight, GL_RGBA32F);
    m_ambientOcclusionBufferTextureId = createImageBuffer(windowWidth, windowHeight, GL_R32F);
    m_ambientOcclusionBufferFilterTargetTextureId = createImageBuffer(windowWidth, windowHeight, GL_R32F);
    m_depthBufferTextureId = createImageBuffer(windowWidth, windowHeight, GL_R32F);
    m_normalBufferTextureId = createImageBuffer(windowWidth, windowHeight, GL_R8UI);
    m_voxelBufferTextureId = createImageBuffer(windowWidth, windowHeight, GL_RGBA16I);
    m_emissionBufferTextureId = createImageBuffer(windowWidth, windowHeight, GL_RGBA32F);
}

void VolumeRenderer::reloadVolumeComputeShader() {
    computeShaderProgram.cleanUp();

    computeShaderProgram.init();
    computeShaderProgram.createComputeShader("../resources/shaders/volumerenderer/volume_ray_marching.glsl");
    computeShaderProgram.link();

    computeShaderProgram.createUniform("tex_volume");
    computeShaderProgram.createUniform("tex_sdf");
    computeShaderProgram.createUniform("volumeDimension");

    computeShaderProgram.createUniform("ray00");
    computeShaderProgram.createUniform("ray01");
    computeShaderProgram.createUniform("ray10");
    computeShaderProgram.createUniform("ray11");
    computeShaderProgram.createUniform("rayOrigin");

    computeShaderProgram.createUniform("udbgSdf");
    computeShaderProgram.createUniform("udbgSdfValue");

    computeShaderProgram.createUniform("MVP");
    ShaderProgram::unbind();
}

GLuint VolumeRenderer::createImageBuffer(int windowWidth, int windowHeight, int imageFormat) {
    GLuint imageBufferId = 0;
    glGenTextures(1, &imageBufferId);
    glBindTexture(GL_TEXTURE_2D, imageBufferId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexStorage2D(GL_TEXTURE_2D, 1, imageFormat, windowWidth, windowHeight);
    glBindTexture(GL_TEXTURE_2D, 0);
    return imageBufferId;
}

void VolumeRenderer::executeBilateralFilter(int width, int height) {
    m_bilateralFilter.execute(m_ambientOcclusionBufferTextureId, m_voxelBufferTextureId, m_normalBufferTextureId,
                              m_positionBufferTextureId, width, height, m_ambientOcclusionBufferFilterTargetTextureId);
    swapAmbientOcclusionBuffers();
}

void VolumeRenderer::reloadVertexFragmentShader() {
    shaderProgram.cleanUp();

    shaderProgram.init();
    shaderProgram.createVertexShader("../resources/shaders/volumerenderer/vertex.glsl");
    shaderProgram.createFragmentShader("../resources/shaders/volumerenderer/fragment.glsl");
    shaderProgram.link();
    shaderProgram.createUniform("tex_albedoBuffer");
    shaderProgram.createUniform("tex_ambientOcclusionBuffer");
    shaderProgram.createUniform("tex_emissionBuffer");
    shaderProgram.createUniform("tex_normalBuffer");
    shaderProgram.createUniform("tex_depthBuffer");
    shaderProgram.createUniform("ambientOcclusionEnabled");
    shaderProgram.createUniform("emissionEnabled");
    shaderProgram.createUniform("emissionFactor");
    shaderProgram.createUniform("dbgMode");
    shaderProgram.createUniform("nearPlane");
    shaderProgram.createUniform("farPlane");
    ShaderProgram::unbind();
}

void VolumeRenderer::swapAmbientOcclusionBuffers() {
    GLuint swap = m_ambientOcclusionBufferTextureId;
    m_ambientOcclusionBufferTextureId = m_ambientOcclusionBufferFilterTargetTextureId;
    m_ambientOcclusionBufferFilterTargetTextureId = swap;
}
