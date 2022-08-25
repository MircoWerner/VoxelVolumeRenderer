#ifndef RENDERENGINE_COMPUTESHADERPROGRAM_H
#define RENDERENGINE_COMPUTESHADERPROGRAM_H

#include <GL/glew.h>

#include "AShaderProgram.h"

/**
 * Compute shader program.
 * Call #init, #createComputeShader, #link for initialization.
 * Execute shader by calling #dispatch.
 *
 * @author Mirco Werner
 */
class ComputeShaderProgram : public AShaderProgram {
public:
    void init();

    void createComputeShader(const std::string &fileName);

    void link();

    void dispatch(int globalWorkSizeX, int globalWorkSizeY = 1, int globalWorkSizeZ = 1);

private:
    int computeShaderId = 0;

    GLint m_localWorkGroupSize[3] = {};
};

#endif