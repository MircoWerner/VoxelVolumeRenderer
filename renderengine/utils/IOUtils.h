#ifndef RENDERENGINE_IOUTILS_H
#define RENDERENGINE_IOUTILS_H

#include "GL/glew.h"

#include <filesystem>

/**
 * Utility methods to read/write files.
 *
 * @author Mirco Werner
 */
class IOUtils {
public:
    static void readAllLines(const std::string& path, std::string *buffer);

    static void writeFramebufferToFile(const std::string& file, int width, int height);
};

#endif