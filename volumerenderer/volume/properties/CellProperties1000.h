#ifndef VOXELVOLUMERENDERER_CELLPROPERTIES1000_H
#define VOXELVOLUMERENDERER_CELLPROPERTIES1000_H

#include "ACellProperties.h"

/**
 * Cell properties for the 1000^3 data set.
 *
 * @author Mirco Werner
 */
class CellProperties1000 : public ACellProperties {
public:
    CellProperties1000(std::string path);

private:
    void generateProperties() override;

    static double random(glm::vec2 st);

    static glm::vec3 getColor(int id);
};

#endif
