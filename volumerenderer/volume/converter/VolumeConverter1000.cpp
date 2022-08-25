#include "VolumeConverter1000.h"

#include <string>

bool VolumeConverter1000::parseCSVLine(std::istringstream &iss, uint32_t *cellId, uint8_t *type) {
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

    if (std::getline(iss, field, ' ')) { // type
        *type = static_cast<uint8_t>(std::stoll(field));
    } else {
        return false;
    }

    return true;
}

bool VolumeConverter1000::excludeType(uint8_t type) {
    return type == 3 || type == 4;
}

bool VolumeConverter1000::suppressCellIdNotInCSVWarning(uint32_t cellId) {
    return false;
}

