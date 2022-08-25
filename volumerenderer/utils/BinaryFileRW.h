#ifndef VOXELVOLUMERENDERER_BINARYFILERW_H
#define VOXELVOLUMERENDERER_BINARYFILERW_H

#include <string>
#include "glm/ext.hpp"

/**
 * IO for binary files.
 *
 * @author Mirco Werner
 */
class BinaryFileRW {
public:
    static bool exists(const std::string &file);

    static void writeBytes(const std::string &file, int bytes, void *data);

    static void readBytes(const std::string &file, int bytes, void *data);
};

#endif