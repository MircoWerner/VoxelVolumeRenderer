#include "VolumeConverter400.h"

#include <glm/ext.hpp>

#include <string>

bool VolumeConverter400::parseCSVLine(std::istringstream &iss, uint32_t *cellId, uint8_t *type) {
    std::string field;

    if (std::getline(iss, field, ' ')) {
        *cellId = static_cast<uint32_t>(std::stoll(field));
    } else {
        return false;
    }

    std::getline(iss, field, ' '); // centerX
    std::getline(iss, field, ' '); // centerY
    std::getline(iss, field, ' '); // centerZ
    std::getline(iss, field, ' '); // volume
    std::getline(iss, field, ' '); // surface
    std::getline(iss, field, ' '); // type
    std::getline(iss, field, ' '); // signal0
    std::getline(iss, field, ' '); // signal1
    std::getline(iss, field, ' '); // signal2
    std::getline(iss, field, ' '); // age
    std::getline(iss, field, ' '); // polarityx
    std::getline(iss, field, ' '); // polarityy
    std::getline(iss, field, ' '); // polarityz
    std::getline(iss, field, ' '); // molitityx
    std::getline(iss, field, ' '); // molitityy
    std::getline(iss, field, ' '); // molitityz
    std::getline(iss, field, ' '); // curvature
    std::getline(iss, field, ' '); // r
    std::getline(iss, field, ' '); // phi
    std::getline(iss, field, ' '); // theta

    if (std::getline(iss, field, ' ')) { // nndistance
        *type = scalarToUInt8(std::stof(field), 6.f, 37.f);
    } else {
        return false;
    }

    return true;
}

bool VolumeConverter400::excludeType(uint8_t type) {
    return type == 1 || type == 6;
}

bool VolumeConverter400::suppressCellIdNotInCSVWarning(uint32_t cellId) {
    return cellId == 1 || cellId == 6;
}

uint8_t VolumeConverter400::scalarToUInt8(float scalar, float min, float max) {
    return static_cast<uint8_t>(glm::smoothstep(min, max, scalar) * 255);
}
