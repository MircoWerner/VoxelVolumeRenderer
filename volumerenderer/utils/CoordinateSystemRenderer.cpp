#include "CoordinateSystemRenderer.h"

#include "../../renderengine/utils/Transformation.h"

void CoordinateSystemRenderer::init() {
    // shader programs
    {
        shaderProgram.init();
        shaderProgram.createVertexShader("../resources/shaders/volumerenderer/utils/coordinate_vertex.glsl");
        shaderProgram.createFragmentShader("../resources/shaders/volumerenderer/utils/coordinate_fragment.glsl");
        shaderProgram.link();
        shaderProgram.createUniform("MVP");
        ShaderProgram::unbind();
    }

    // vertex data
    {
        std::vector<float> vertices = {
                0.1, -0.1, 0.1, 0.1, 0.1, 0.1, 0.1, -0.1, -0.1, 0.1, 0.1, -0.1, 1.1, -0.1, 0.1, 1.1, 0.1, 0.1, 1.1,
                -0.1, -0.1, 1.1, 0.1, -0.1, -0.1, 0.1, 0.1, -0.1, 1.1, 0.1, -0.1, 0.1, -0.1, -0.1, 1.1, -0.1, 0.1, 0.1,
                0.1, 0.1, 1.1, 0.1, 0.1, 0.1, -0.1, 0.1, 1.1, -0.1, -0.1, -0.1, 1.1, -0.1, 0.1, 1.1, -0.1, -0.1, 0.1,
                -0.1, 0.1, 0.1, 0.1, -0.1, 1.1, 0.1, 0.1, 1.1, 0.1, -0.1, 0.1, 0.1, 0.1, 0.1,
        };
        std::vector<float> colors = {
                1.f, 0.f, 0.f,
                1.f, 0.f, 0.f,
                1.f, 0.f, 0.f,
                1.f, 0.f, 0.f,
                1.f, 0.f, 0.f,
                1.f, 0.f, 0.f,
                1.f, 0.f, 0.f,
                1.f, 0.f, 0.f,

                0.f, 1.f, 0.f,
                0.f, 1.f, 0.f,
                0.f, 1.f, 0.f,
                0.f, 1.f, 0.f,
                0.f, 1.f, 0.f,
                0.f, 1.f, 0.f,
                0.f, 1.f, 0.f,
                0.f, 1.f, 0.f,

                0.f, 0.f, 1.f,
                0.f, 0.f, 1.f,
                0.f, 0.f, 1.f,
                0.f, 0.f, 1.f,
                0.f, 0.f, 1.f,
                0.f, 0.f, 1.f,
                0.f, 0.f, 1.f,
                0.f, 0.f, 1.f,
        };
        std::vector<int> indices = {
                1, 2, 0, 3, 6, 2, 7, 4, 6, 5, 0, 4, 6, 0, 2, 3, 5, 7, 1, 3, 2, 3, 7, 6, 7, 5, 4, 5, 1, 0, 6, 4, 0, 3, 1,
                5, 9, 10, 8, 11, 14, 10, 15, 12, 14, 13, 8, 12, 14, 8, 10, 11, 13, 15, 9, 11, 10, 11, 15, 14, 15, 13,
                12, 13, 9, 8, 14, 12, 8, 11, 9, 13, 17, 18, 16, 19, 22, 18, 23, 20, 22, 21, 16, 20, 22, 16, 18, 19, 21,
                23, 17, 19, 18, 19, 23, 22, 23, 21, 20, 21, 17, 16, 22, 20, 16, 19, 17, 21,
        };
        count = indices.size();

        glGenVertexArrays(1, &vaoId);
        glBindVertexArray(vaoId);

        glGenBuffers(1, &vertexVboId);
        glBindBuffer(GL_ARRAY_BUFFER, vertexVboId);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);

        glGenBuffers(1, &colorVboId);
        glBindBuffer(GL_ARRAY_BUFFER, colorVboId);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, nullptr);

        glGenBuffers(1, &indicesVboId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesVboId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void CoordinateSystemRenderer::render(ACamera *camera, int windowWidth, int windowHeight) {
    if (!m_render) {
        return;
    }

    shaderProgram.bind();

    static const float FOV = glm::radians(90.0f);
    static const float Z_NEAR = 0.01f;
    static const float Z_FAR = 1000.f;
    glm::mat4 modelMatrix = Transformation::getModelMatrix(glm::vec3(0.f, 0.f, -1.f), camera->getRotation(),
                                                           glm::vec3(0.1f));
    glm::mat4 translationMatrix = Transformation::getModelMatrix(glm::vec3(0.85f, 0.85f, 0.f), glm::vec3(0.f),
                                                                 glm::vec3(1.f));
    glm::mat4 MVP = translationMatrix * Transformation::getProjectionMatrix(FOV, static_cast<float>(windowWidth),
                                                                            static_cast<float>(windowHeight), Z_NEAR,
                                                                            Z_FAR) * modelMatrix;

    shaderProgram.setUniform("MVP", MVP);

    glBindVertexArray(vaoId);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindVertexArray(0);

    ShaderProgram::unbind();
}

void CoordinateSystemRenderer::cleanUp() {
    glDeleteBuffers(1, &vertexVboId);
    glDeleteBuffers(1, &colorVboId);
    glDeleteBuffers(1, &indicesVboId);
    glDeleteVertexArrays(1, &vaoId);
    shaderProgram.cleanUp();
}
