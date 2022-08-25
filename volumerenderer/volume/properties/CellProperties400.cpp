#include "CellProperties400.h"

CellProperties400::CellProperties400(std::string path) : ACellProperties(std::move(path), 256) {

}

void CellProperties400::generateProperties() {
    for (int i = 0; i < m_idCount; i++) {
        CProperties properties{};
        properties.id = i;
        properties.valid = 1;
        glm::vec3 col = glm::vec3(m_COOL_WARM[i * 3 + 0], m_COOL_WARM[i * 3 + 1],
                                  m_COOL_WARM[i * 3 + 2]);
        properties.rgba[0] = col.r;
        properties.rgba[1] = col.g;
        properties.rgba[2] = col.b;
        properties.rgba[3] = 1.f;
        properties.emittance = 1.f;
        properties.roughness = 0.5f;
        m_properties[i] = properties;
    }
}
