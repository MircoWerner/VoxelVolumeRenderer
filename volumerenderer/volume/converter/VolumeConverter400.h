#ifndef VOXELVOLUMERENDERER_VOLUMECONVERTER400_H
#define VOXELVOLUMERENDERER_VOLUMECONVERTER400_H

#include "AVolumeConverter.h"

/**
 * Converter for the 400^3 data set.
 *
 * @author Mirco Werner
 */
class VolumeConverter400 : public AVolumeConverter<uint32_t> {
private:
    bool parseCSVLine(std::istringstream &iss, uint32_t *cellId, uint8_t *type) override;

    bool excludeType(uint8_t type) override;

    bool suppressCellIdNotInCSVWarning(uint32_t cellId) override;

    static uint8_t scalarToUInt8(float scalar, float min, float max);
};

#endif
