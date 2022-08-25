/**
 * Second step of the Directional RGB Distance Field calculation.
 * Propagates the emitted light.
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, rgba32ui) uniform uimage3D df;

uniform usampler3D tex_volume;

uniform ivec3 rgbDimension;// dimension of the rgb df buffer
uniform ivec3 displacement;// displacement in the rgb df buffer

uniform int zDisplacement;// displacement in the volume

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

// dxyz \in {-1,0,1}^3
int toBufferIdx(ivec3 dxyz) {
    return (dxyz.x + 1) * 9 + (dxyz.y + 1) * 3 + (dxyz.z + 1) - 1;
}

uvec4 getDF(ivec3 voxel) {
    return imageLoad(df, voxel);
}

void setDF(ivec3 voxel, uvec4 value) {
    imageStore(df, voxel, value);
}

uint getValue(uvec4 dfValue, int bufferIdx) {
    int componentIdx = bufferIdx / 8;// each rgba-component (rgba32ui) stores 8 entries (4 byte, half byte per entry)
    int innerIdx = bufferIdx % 8;

    uint component = dfValue[componentIdx];
    component = component >> (innerIdx * 4);// shift relevant part to lower 4 bits
    component = component & uint(0xF);// masking

    return component;
}

void setValue(inout uvec4 dfValue, int bufferIdx, uint value) {
    int componentIdx = bufferIdx / 8;// each rgba-component (rgba32ui) stores 8 entries (4 byte, half byte per entry)
    int innerIdx = bufferIdx % 8;

    uint encoded = value << (innerIdx * 4);// shift relevant part back to correct position
    uint deleteMask = ~(uint(0xF) << (innerIdx * 4));// zeroes for the relevant part

    uint component = dfValue[componentIdx];
    component = component & deleteMask;// delete old value
    component = component | encoded;// store new value

    dfValue[componentIdx] = component;
}

bool isVoxelSolid(ivec3 voxel) {
    return texelFetch(tex_volume, voxel, 0).r > 0;
}

uint isSolid(ivec3 voxel) {
    return texelFetch(tex_volume, voxel, 0).r > 0 ? 0 : 1;
}

float lightBlockFactor(ivec3 voxel, int dx, int dy, int dz) {
    uint s_dxy_ = isSolid(voxel + ivec3(dx, dy, 0));
    uint s_dx_z = isSolid(voxel + ivec3(dx, 0, dz));
    uint s_d_yz = isSolid(voxel + ivec3(0, dy, dz));
    uint s_dx = isSolid(voxel + ivec3(dx, 0, 0));
    uint s_dy = isSolid(voxel + ivec3(0, dy, 0));
    uint s_dz = isSolid(voxel + ivec3(0, 0, dz));

    return 1 - float(s_dxy_ * (s_dx + s_dy) + s_dx_z * (s_dx + s_dz) + s_d_yz * (s_dy + s_dz)) / 6.0;
}


    #define STRAIGHT_DECAY 1
void dx__(inout uvec4 currentVoxelDF, ivec3 voxel, int dx) {
    ivec3 dxyz = ivec3(dx, 0, 0);

    uvec4 otherVoxel = getDF(voxel + dxyz);

    uint valM1M1 = getValue(otherVoxel, toBufferIdx(ivec3(dx, -1, -1)));
    uint valM10 = getValue(otherVoxel, toBufferIdx(ivec3(dx, -1, 0)));
    uint valM11 = getValue(otherVoxel, toBufferIdx(ivec3(dx, -1, 1)));

    uint val0M1 = getValue(otherVoxel, toBufferIdx(ivec3(dx, 0, -1)));
    uint val00 = getValue(otherVoxel, toBufferIdx(ivec3(dx, 0, 0)));
    uint val01 = getValue(otherVoxel, toBufferIdx(ivec3(dx, 0, 1)));

    uint val1M1 = getValue(otherVoxel, toBufferIdx(ivec3(dx, 1, -1)));
    uint val10 = getValue(otherVoxel, toBufferIdx(ivec3(dx, 1, 0)));
    uint val11 = getValue(otherVoxel, toBufferIdx(ivec3(dx, 1, 1)));

    uint maxValue = max(valM1M1, max(valM10, max(valM11, max(val0M1, max(val00, max(val01, max(val1M1, max(val10, val11))))))));

    maxValue = maxValue >= STRAIGHT_DECAY ? maxValue - STRAIGHT_DECAY : 0;

    setValue(currentVoxelDF, toBufferIdx(dxyz), maxValue);
}
void d_y_(inout uvec4 currentVoxelDF, ivec3 voxel, int dy) {
    ivec3 dxyz = ivec3(0, dy, 0);

    uvec4 otherVoxel = getDF(voxel + dxyz);

    uint valM1M1 = getValue(otherVoxel, toBufferIdx(ivec3(-1, dy, -1)));
    uint valM10 = getValue(otherVoxel, toBufferIdx(ivec3(-1, dy, 0)));
    uint valM11 = getValue(otherVoxel, toBufferIdx(ivec3(-1, dy, 1)));

    uint val0M1 = getValue(otherVoxel, toBufferIdx(ivec3(0, dy, -1)));
    uint val00 = getValue(otherVoxel, toBufferIdx(ivec3(0, dy, 0)));
    uint val01 = getValue(otherVoxel, toBufferIdx(ivec3(0, dy, 1)));

    uint val1M1 = getValue(otherVoxel, toBufferIdx(ivec3(1, dy, -1)));
    uint val10 = getValue(otherVoxel, toBufferIdx(ivec3(1, dy, 0)));
    uint val11 = getValue(otherVoxel, toBufferIdx(ivec3(1, dy, 1)));

    uint maxValue = max(valM1M1, max(valM10, max(valM11, max(val0M1, max(val00, max(val01, max(val1M1, max(val10, val11))))))));

    maxValue = maxValue >= STRAIGHT_DECAY ? maxValue - STRAIGHT_DECAY : 0;

    setValue(currentVoxelDF, toBufferIdx(dxyz), maxValue);
}
void d__z(inout uvec4 currentVoxelDF, ivec3 voxel, int dz) {
    ivec3 dxyz = ivec3(0, 0, dz);

    uvec4 otherVoxel = getDF(voxel + dxyz);

    uint valM1M1 = getValue(otherVoxel, toBufferIdx(ivec3(-1, -1, dz)));
    uint valM10 = getValue(otherVoxel, toBufferIdx(ivec3(-1, 0, dz)));
    uint valM11 = getValue(otherVoxel, toBufferIdx(ivec3(-1, 1, dz)));

    uint val0M1 = getValue(otherVoxel, toBufferIdx(ivec3(0, -1, dz)));
    uint val00 = getValue(otherVoxel, toBufferIdx(ivec3(0, 0, dz)));
    uint val01 = getValue(otherVoxel, toBufferIdx(ivec3(0, 1, dz)));

    uint val1M1 = getValue(otherVoxel, toBufferIdx(ivec3(1, -1, dz)));
    uint val10 = getValue(otherVoxel, toBufferIdx(ivec3(1, 0, dz)));
    uint val11 = getValue(otherVoxel, toBufferIdx(ivec3(1, 1, dz)));

    uint maxValue = max(valM1M1, max(valM10, max(valM11, max(val0M1, max(val00, max(val01, max(val1M1, max(val10, val11))))))));

    maxValue = maxValue >= STRAIGHT_DECAY ? maxValue - STRAIGHT_DECAY : 0;

    setValue(currentVoxelDF, toBufferIdx(dxyz), maxValue);
}

    #define DIAGONAL_DECAY 2
void dxy_(inout uvec4 currentVoxelDF, ivec3 voxel, int dx, int dy) {
    ivec3 dxyz = ivec3(dx, dy, 0);

    uvec4 otherVoxel = getDF(voxel + dxyz);

    uint valM1 = getValue(otherVoxel, toBufferIdx(ivec3(dx, dy, -1)));
    uint val0 = getValue(otherVoxel, toBufferIdx(ivec3(dx, dy, 0)));
    uint val1 = getValue(otherVoxel, toBufferIdx(ivec3(dx, dy, 1)));

    uint maxValue = max(valM1, max(val0, val1));

    float xSolid = isVoxelSolid(voxel + ivec3(dx, 0, 0) + ivec3(0, 0, zDisplacement)) ? 0.5 : 0;
    float ySolid = isVoxelSolid(voxel + ivec3(0, dy, 0) + ivec3(0, 0, zDisplacement)) ? 0.5 : 0;
    maxValue = uint(ceil(maxValue * (1.0 - xSolid - ySolid)));

    maxValue = maxValue >= DIAGONAL_DECAY ? maxValue - DIAGONAL_DECAY : 0;

    setValue(currentVoxelDF, toBufferIdx(dxyz), maxValue);
}
void d_yz(inout uvec4 currentVoxelDF, ivec3 voxel, int dy, int dz) {
    ivec3 dxyz = ivec3(0, dy, dz);

    uvec4 otherVoxel = getDF(voxel + dxyz);

    uint valM1 = getValue(otherVoxel, toBufferIdx(ivec3(-1, dy, dz)));
    uint val0 = getValue(otherVoxel, toBufferIdx(ivec3(0, dy, dz)));
    uint val1 = getValue(otherVoxel, toBufferIdx(ivec3(1, dy, dz)));

    uint maxValue = max(valM1, max(val0, val1));

    float ySolid = isVoxelSolid(voxel + ivec3(0, dy, 0) + ivec3(0, 0, zDisplacement)) ? 0.5 : 0;
    float zSolid = isVoxelSolid(voxel + ivec3(0, 0, dz) + ivec3(0, 0, zDisplacement)) ? 0.5 : 0;
    maxValue = uint(ceil(maxValue * (1.0 - ySolid - zSolid)));

    maxValue = maxValue >= DIAGONAL_DECAY ? maxValue - DIAGONAL_DECAY : 0;

    setValue(currentVoxelDF, toBufferIdx(dxyz), maxValue);
}
void dx_z(inout uvec4 currentVoxelDF, ivec3 voxel, int dx, int dz) {
    ivec3 dxyz = ivec3(dx, 0, dz);

    uvec4 otherVoxel = getDF(voxel + dxyz);

    uint valM1 = getValue(otherVoxel, toBufferIdx(ivec3(dx, -1, dz)));
    uint val0 = getValue(otherVoxel, toBufferIdx(ivec3(dx, 0, dz)));
    uint val1 = getValue(otherVoxel, toBufferIdx(ivec3(dx, 1, dz)));

    uint maxValue = max(valM1, max(val0, val1));

    float xSolid = isVoxelSolid(voxel + ivec3(dx, 0, 0) + ivec3(0, 0, zDisplacement)) ? 0.5 : 0;
    float zSolid = isVoxelSolid(voxel + ivec3(0, 0, dz) + ivec3(0, 0, zDisplacement)) ? 0.5 : 0;
    maxValue = uint(ceil(maxValue * (1.0 - xSolid - zSolid)));

    maxValue = maxValue >= DIAGONAL_DECAY ? maxValue - DIAGONAL_DECAY : 0;

    setValue(currentVoxelDF, toBufferIdx(dxyz), maxValue);
}

    #define CORNER_DECAY 3
void dxyz(inout uvec4 currentVoxelDF, ivec3 voxel, int dx, int dy, int dz) {
    ivec3 dxyz = ivec3(dx, dy, dz);

    uvec4 otherVoxel = getDF(voxel + dxyz);

    uint maxValue = getValue(otherVoxel, toBufferIdx(ivec3(dx, dy, dz)));

    maxValue = uint(ceil(maxValue * lightBlockFactor(voxel, dx, dy, dz)));

    maxValue = maxValue >= CORNER_DECAY ? maxValue - CORNER_DECAY : 0;

    setValue(currentVoxelDF, toBufferIdx(dxyz), maxValue);
}



void main() {
    ivec3 voxel = ivec3(gl_GlobalInvocationID.xyz) + displacement;
    if (voxel.x >= rgbDimension.x || voxel.y >= rgbDimension.y || voxel.z >= rgbDimension.z) {
        return;
    }

    if (isVoxelSolid(voxel + ivec3(0, 0, zDisplacement))) {
        return;
    }

    uvec4 currentVoxelDF = getDF(voxel);

    {
        // === STRAIGHT LIGHT (DECAY 1) (6 entries)
        // x-axis
        dx__(currentVoxelDF, voxel, -1);
        dx__(currentVoxelDF, voxel, 1);
        // y-axis
        d_y_(currentVoxelDF, voxel, -1);
        d_y_(currentVoxelDF, voxel, 1);
        // z-axis
        d__z(currentVoxelDF, voxel, -1);
        d__z(currentVoxelDF, voxel, 1);
    }
    {
        //  === DIAGONAL LIGHT (DECAY 2) (12 entries)
        // xy-axis
        dxy_(currentVoxelDF, voxel, -1, -1);
        dxy_(currentVoxelDF, voxel, -1, 1);
        dxy_(currentVoxelDF, voxel, 1, -1);
        dxy_(currentVoxelDF, voxel, 1, 1);
        // yz-axis
        d_yz(currentVoxelDF, voxel, -1, -1);
        d_yz(currentVoxelDF, voxel, -1, 1);
        d_yz(currentVoxelDF, voxel, 1, -1);
        d_yz(currentVoxelDF, voxel, 1, 1);
        // xz-axis
        dx_z(currentVoxelDF, voxel, -1, -1);
        dx_z(currentVoxelDF, voxel, -1, 1);
        dx_z(currentVoxelDF, voxel, 1, -1);
        dx_z(currentVoxelDF, voxel, 1, 1);
    }
    {
        // === CORNER LIGHT (DECAY 3) (8 entries)
        dxyz(currentVoxelDF, voxel, -1, -1, -1);
        dxyz(currentVoxelDF, voxel, -1, -1, 1);
        dxyz(currentVoxelDF, voxel, -1, 1, -1);
        dxyz(currentVoxelDF, voxel, -1, 1, 1);
        dxyz(currentVoxelDF, voxel, 1, -1, -1);
        dxyz(currentVoxelDF, voxel, 1, -1, 1);
        dxyz(currentVoxelDF, voxel, 1, 1, -1);
        dxyz(currentVoxelDF, voxel, 1, 1, 1);
    }

    setDF(voxel, currentVoxelDF);
}