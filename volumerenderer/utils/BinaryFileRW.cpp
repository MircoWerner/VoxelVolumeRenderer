#include "BinaryFileRW.h"

#include <fstream>
#include <sstream>

void BinaryFileRW::writeBytes(const std::string &file, int bytes, void *data) {
    std::ofstream handle(file, std::ios_base::out | std::ios_base::binary);
    if (!handle.is_open()) {
        std::ostringstream err;
        err << "Unable to open file at: " << file << "\n";
        throw std::runtime_error(err.str());
    }
    handle.write(static_cast<char *>(data), bytes);
    handle.close();
}

void BinaryFileRW::readBytes(const std::string &file, int bytes, void *data) {
    std::ifstream handle(file, std::ios_base::in | std::ios_base::binary);
    if (!handle.is_open()) {
        std::ostringstream err;
        err << "Unable to open file at: " << file << "\n";
        throw std::runtime_error(err.str());
    }
    handle.read(static_cast<char *>(data), bytes);
    handle.close();
}

bool BinaryFileRW::exists(const std::string &file) {
    std::ifstream handle(file);
    return handle.good();
}
