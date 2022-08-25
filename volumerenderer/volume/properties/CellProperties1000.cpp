#include "CellProperties1000.h"

CellProperties1000::CellProperties1000(std::string path) : ACellProperties(std::move(path), 28) {

}

void CellProperties1000::generateProperties() {
    for (int i = 0; i < m_idCount; i++) {
        glm::vec3 color = getColor(i);
        CProperties properties{};
        properties.id = i;
        properties.valid = 1;
        properties.rgba[0] = color.r;
        properties.rgba[1] = color.g;
        properties.rgba[2] = color.b;
        properties.rgba[3] = 1.f;
        properties.emittance = (i == 21 || i == 27) ? 0.f : 1.f;
        properties.roughness = 0.5f;
        m_properties[i] = properties;
    }
}

double CellProperties1000::random(glm::vec2 st) {
    // https://thebookofshaders.com/10/
    return glm::fract(glm::sin(glm::dot(st, glm::vec2(12.9898, 78.233))) * 43758.5453123);
}

glm::vec3 CellProperties1000::getColor(int id) {
    glm::vec3 rgb;
    switch (id) {
        case 0:
            rgb = glm::vec3(0);
            break;
        case 1:
            rgb = glm::vec3(145, 0, 0) / 255.f;
            break;
        case 21:
            rgb = glm::vec3(235, 237, 182) / 255.f;
            break;
        case 27:
            rgb = glm::vec3(70, 76, 90) / 255.f;
            break;
        default:
            auto idf = static_cast<float>(id);
            rgb = glm::vec3(random(glm::vec2(idf, idf)),
                            random(glm::vec2(idf, 2 * idf)),
                            random(glm::vec2(2 * idf, idf)));
            break;
    }
    return rgb;
}