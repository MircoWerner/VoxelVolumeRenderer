#ifndef VOXELVOLUMERENDERER_AVOLUMECONVERTER_H
#define VOXELVOLUMERENDERER_AVOLUMECONVERTER_H

#include <string>
#include <unordered_map>
#include <vector>

#include <iostream>
#include <thread>
#include <fstream>
#include <sstream>

#include <climits>

/**
 * Converts the volume data.
 *
 * @author Mirco Werner
 *
 * @tparam T uint16_t / uint32_t, bytes per voxel in the volume file
 */
template<typename T>
class AVolumeConverter {
public:
    /**
     * Multithreaded conversion of the data.
     *
     * @param pathVolume path to the volume file (*.raw)
     * @param pathCSV path to the csv property file (*.csv)
     * @param volume out parameter: array will be filled with the converted data
     * @param margin to reduce the dimension of the volume
     */
    void
    loadVolume(const std::string &pathVolume, const std::string &pathCSV, std::vector<uint8_t> *volume, int margin);

    int m_volumeDimensionX = 0;
    int m_volumeDimensionY = 0;
    int m_volumeDimensionZ = 0;

private:
    void
    convertVolumePart(T *data, std::vector<uint8_t> *volume, std::unordered_map<T, uint8_t> *types, int zStart,
                      int zEnd, int margin, int volumeDimensionX, int volumeDimensionY, int volumeDimensionZ);

    void loadCSV(const std::string &pathCSV, std::unordered_map<T, uint8_t> *types);

    virtual bool parseCSVLine(std::istringstream &iss, uint32_t *cellId, uint8_t *type) = 0;

    virtual bool excludeType(uint8_t type) = 0;

    virtual bool suppressCellIdNotInCSVWarning(T cellId) = 0;
};

template<typename T>
void
AVolumeConverter<T>::loadVolume(const std::string &pathVolume, const std::string &pathCSV, std::vector<uint8_t> *volume,
                                int margin) {
    std::unordered_map<T, uint8_t> types;
    loadCSV(pathCSV, &types);

    // begin
    std::ifstream volumeFile(pathVolume, std::ios_base::in | std::ios_base::binary);
    if (!volumeFile.is_open()) {
        throw std::runtime_error("Unable to open volume file.");
    }

    // read dimension
    int realVolumeDimensionX, realVolumeDimensionY, realVolumeDimensionZ;
    volumeFile >> realVolumeDimensionX >> realVolumeDimensionY >> realVolumeDimensionZ;
    volumeFile.get(); // skip whitespace
    m_volumeDimensionX = realVolumeDimensionX - 2 * margin;
    m_volumeDimensionY = realVolumeDimensionY - 2 * margin;
    m_volumeDimensionZ = realVolumeDimensionZ - 2 * margin;
    std::cout << realVolumeDimensionX << "," << realVolumeDimensionY << "," << realVolumeDimensionZ << " (margin="
              << margin << " => " << m_volumeDimensionX << "," << m_volumeDimensionY << "," << m_volumeDimensionZ
              << ")" << std::endl;


    // read data type
    std::string type;
    std::getline(volumeFile, type);
    std::cout << type << std::endl;

    uint16_t bits_per_sample = 0;
    if (type == "uint16") {
        bits_per_sample = 16;
    } else if (type == "uint32") {
        bits_per_sample = 32;
    }

    if (bits_per_sample != 16 && bits_per_sample != 32) {
        volumeFile.close();
        throw std::runtime_error("Expected precision of 16bit or 32bit per sample in volume file.");
    }

    const uint64_t MAX_ALLOWED_VOXELS = 2048ull * 2048ull * 2048ull;
    const uint64_t voxelCount = realVolumeDimensionX * realVolumeDimensionY * realVolumeDimensionZ;

    if (MAX_ALLOWED_VOXELS < voxelCount) {
        volumeFile.close();
        throw std::invalid_argument("Volume exceeds maximum allowed size.");
    }

    size_t byteSize = voxelCount * (bits_per_sample / 8);
    char *payload = new char[byteSize];

    // read binary data
    size_t bytesPerRead = 1 << 28;
    size_t bytesRead = 0;
    char *currPayload = payload;
    while (volumeFile) {
        volumeFile.read(currPayload, std::min(bytesPerRead, byteSize - bytesRead));
        if (!volumeFile.gcount()) {
            break;
        }
        bytesRead += bytesPerRead;
        currPayload += bytesPerRead;
    }
    volumeFile.close();

    // convert
    volume->resize(m_volumeDimensionX * m_volumeDimensionY * m_volumeDimensionZ, 0);

    T *data = (T *) payload;

    const int NUM_THREADS = 8;
    const int Z_LEVELS_PER_THREAD = realVolumeDimensionZ / NUM_THREADS;

    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; i++) {
        int zStart = i * Z_LEVELS_PER_THREAD;
        int zEnd = (i == NUM_THREADS - 1) ? realVolumeDimensionZ : ((i + 1) * Z_LEVELS_PER_THREAD);
        threads.emplace_back(&AVolumeConverter::convertVolumePart, this, data, volume, &types, zStart, zEnd, margin,
                             m_volumeDimensionX,
                             m_volumeDimensionY, m_volumeDimensionZ);
    }

    for (auto &thread: threads) {
        thread.join();
    }

    delete[] payload;
}

template<typename T>
void
AVolumeConverter<T>::convertVolumePart(T *data, std::vector<uint8_t> *volume, std::unordered_map<T, uint8_t> *types,
                                       int zStart, int zEnd, int margin, int volumeDimensionX, int volumeDimensionY,
                                       int volumeDimensionZ) {
    int indexStart = zStart * (volumeDimensionX + 2 * margin) * (volumeDimensionY + 2 * margin);
    int indexEnd = zEnd * (volumeDimensionX + 2 * margin) * (volumeDimensionY + 2 * margin);
    int x = 0, y = 0, z = zStart;
    int index = std::max(zStart - margin, 0) * volumeDimensionX * volumeDimensionY;
    for (int i = indexStart; i < indexEnd; i++) {

        if (x >= margin && x < volumeDimensionX + margin
            && y >= margin && y < volumeDimensionY + margin
            && z >= margin && z < volumeDimensionZ + margin) {

            T cellId = data[i];

            if (types->find(cellId) != types->end()) {
                uint8_t tp = types->at(cellId);
                if (excludeType(tp)) {
                    (*volume)[index] = 0;
                } else {
                    (*volume)[index] = tp;
                }
            } else {
                if (!suppressCellIdNotInCSVWarning(cellId)) {
                    std::cerr << "Did not find " << cellId << ". Set voxel to AIR." << std::endl;
                }
                (*volume)[index] = 0;
            }

            index++;
        }

        x++;
        if (x >= volumeDimensionX + 2 * margin) {
            x = 0;
            y++;
        }
        if (y >= volumeDimensionY + 2 * margin) {
            y = 0;
            z++;
            std::cout << "[" << zStart << "," << zEnd << "): " << z << " => " << 100 * (z - zStart) / (zEnd - zStart)
                      << "%" << std::endl;
        }
    }
}

template<typename T>
void AVolumeConverter<T>::loadCSV(const std::string &pathCSV, std::unordered_map<T, uint8_t> *types) {
    std::ifstream csv(pathCSV);
    if (!csv.is_open()) {
        throw std::runtime_error("Unable to open csv file.");
    }

    csv.ignore(LLONG_MAX, '\n'); // skip first line

    std::string line;
    while (std::getline(csv, line)) {
        std::istringstream iss(line);
        uint32_t cellId;
        uint8_t type;
        if (parseCSVLine(iss, &cellId, &type)) {
            (*types)[cellId] = type;
        } else {
            std::cout << "Cannot parse csv line: " << line << std::endl;
        }
    }

    csv.close();

    std::cout << "Found " << types->size() << " cell entries in csv." << std::endl;
}

#endif
