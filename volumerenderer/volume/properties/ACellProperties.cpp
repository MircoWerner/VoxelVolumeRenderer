#include "ACellProperties.h"

#include <random>
#include <iostream>
#include <thread>
#include <fstream>
#include <sstream>
#include <utility>

ACellProperties::ACellProperties(std::string path, int idCount) : m_path(std::move(path)), m_idCount(idCount) {

}

void ACellProperties::loadPropertiesCSV(bool readFromFile, bool writeToFile) {
    m_properties.clear();
    m_properties.resize(m_idCount);

    if (!readFromFile || !std::ifstream(m_path).good()) {
        generateProperties();
        if (writeToFile) {
            writePropertiesCSV();
        }
        return;
    }

    parsePropertiesCSV();
}

void ACellProperties::writePropertiesCSV() {
    std::ofstream outCSV(m_path, std::ios_base::out | std::ios_base::trunc);
    if (!outCSV.is_open()) {
        std::ostringstream err;
        err << "unable to open output file at: " << m_path << "\n";
        throw std::runtime_error(err.str());
    }

    outCSV << m_CELL_PROPERTIES_HEADER << std::endl;

    for (int i = 0; i < m_properties.size(); i++) {
        CProperties properties = m_properties[i];
        std::string str = std::to_string(properties.id) + " " + std::to_string(properties.valid) + " " +
                          std::to_string(properties.rgba[0]) + " " + std::to_string(properties.rgba[1]) + " " +
                          std::to_string(properties.rgba[2]) + " " + std::to_string(properties.rgba[3]) + " " +
                          std::to_string(properties.emittance) + " " + std::to_string(properties.roughness);
        outCSV << str << std::endl;
    }

    outCSV.close();
}

void ACellProperties::parsePropertiesCSV() {
    std::ifstream csv(m_path);
    if (!csv.is_open()) {
        throw std::runtime_error("Unable to open csv file.");
    }

    csv.ignore(LLONG_MAX, '\n'); // skip first line
    std::string line;
    for (int i = 0; std::getline(csv, line) && i < m_properties.size(); i++) {
        std::istringstream iss(line);
        std::string field;

        CProperties properties{};

        if (std::getline(iss, field, ' ')) {
            properties.id = static_cast<uint8_t>(std::stoi(field));
        } else {
            std::cout << "Cannot parse cellID: " << line << std::endl;
            continue;
        }

        uint8_t valid;
        if (std::getline(iss, field, ' ')) {
            properties.valid = static_cast<uint8_t>(std::stoi(field));
        } else {
            std::cout << "Cannot parse valid: " << line << std::endl;
            continue;
        }

        if (std::getline(iss, field, ' ')) {
            properties.rgba[0] = std::stof(field);
        } else {
            std::cout << "Cannot parse r: " << line << std::endl;
            continue;
        }
        if (std::getline(iss, field, ' ')) {
            properties.rgba[1] = std::stof(field);
        } else {
            std::cout << "Cannot parse g: " << line << std::endl;
            continue;
        }
        if (std::getline(iss, field, ' ')) {
            properties.rgba[2] = std::stof(field);
        } else {
            std::cout << "Cannot parse b: " << line << std::endl;
            continue;
        }
        if (std::getline(iss, field, ' ')) {
            properties.rgba[3] = std::stof(field);
        } else {
            std::cout << "Cannot parse a: " << line << std::endl;
            continue;
        }

        if (std::getline(iss, field, ' ')) {
            properties.emittance = std::stof(field);
        } else {
            std::cout << "Cannot parse emittance: " << line << std::endl;
            continue;
        }
        if (std::getline(iss, field, ' ')) {
            properties.roughness = std::stof(field);
        } else {
            std::cout << "Cannot parse roughness: " << line << std::endl;
            continue;
        }

        m_properties[i] = properties;
    }
    csv.close();
}
