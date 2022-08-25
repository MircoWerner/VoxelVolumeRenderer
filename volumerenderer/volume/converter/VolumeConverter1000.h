#ifndef VOXELVOLUMERENDERER_VOLUMECONVERTER1000_H
#define VOXELVOLUMERENDERER_VOLUMECONVERTER1000_H

#include "AVolumeConverter.h"

/**
 * Converter for the 1000^3 data set.
 *
 * @author Mirco Werner
 */
class VolumeConverter1000 : public AVolumeConverter<uint32_t> {
private:
    bool parseCSVLine(std::istringstream &iss, uint32_t *cellId, uint8_t *type) override;

    bool excludeType(uint8_t type) override;

    bool suppressCellIdNotInCSVWarning(uint32_t cellId) override;
};

#endif
