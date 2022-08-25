#ifndef VOXELVOLUMERENDERER_ACELLPROPERTIES_H
#define VOXELVOLUMERENDERER_ACELLPROPERTIES_H

#include <vector>
#include <string>

#include "glm/ext.hpp"

/**
 * Generates or loads cell properties like albedo and emittance value.
 *
 * @author Mirco Werner
 */
class ACellProperties {
public:
    ACellProperties(std::string path, int idCount);

    void loadPropertiesCSV(bool readFromFile, bool writeToFile);

    void writePropertiesCSV();

    struct CProperties {
        uint8_t id;
        uint8_t valid;
        float rgba[4];
        float emittance;
        float roughness;
    };

    std::vector<CProperties> m_properties;

protected:
    const int m_idCount;

private:
    const std::string m_path;

    const std::string m_CELL_PROPERTIES_HEADER = "id valid red green blue alpha emittance roughness";

    virtual void generateProperties() = 0;

    void parsePropertiesCSV();
};

#endif