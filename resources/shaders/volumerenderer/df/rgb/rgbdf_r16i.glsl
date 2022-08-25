/**
 * Second step of the RGB Distance Field calculation.
 * Propagates the emitted light.
 * Reference: https://0fps.net/2018/02/21/voxel-lighting/
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, r16ui) uniform uimage3D df;// [ 1bit alpha | 5bit blue | 5bit green | 5bit red ] - alpha encodes occupancy

uniform usampler3D tex_volume;
uniform ivec3 volumeDimension;
uniform ivec3 displacement;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#define RED_MASK uint(0x001F)
#define GREEN_MASK uint(0x03E0)
#define BLUE_MASK uint(0x7C00)

#define GREEN_SHIFT 5
#define BLUE_SHIFT 10

ivec3 decompressRGB(ivec3 voxel) {
    uint value = imageLoad(df, voxel).r;
    ivec3 rgb;
    rgb.r = int(value & RED_MASK);
    rgb.g = int((value & GREEN_MASK) >> GREEN_SHIFT);
    rgb.b = int((value & BLUE_MASK) >> BLUE_SHIFT);
    return rgb;
}

void compressRGB(ivec3 voxel, uvec3 rgb) {
    uint value = (rgb.b << BLUE_SHIFT) | (rgb.g << GREEN_SHIFT) | (rgb.r);
    imageStore(df, voxel, uvec4(value, 0, 0, 0));
}

bool isVoxelSolid(ivec3 voxel) {
    return texelFetch(tex_volume, voxel, 0).r > 0;
}

ivec3 maxv(ivec3 a, ivec3 b) {
    return ivec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}

    #define DECAY 1
void main() {
    ivec3 voxel = ivec3(gl_GlobalInvocationID.xyz) + displacement;
    if (voxel.x >= volumeDimension.x || voxel.y >= volumeDimension.y || voxel.z >= volumeDimension.z) {
        return;
    }

    if (isVoxelSolid(voxel)) {
        return;
    }

    ivec3 dfXN = voxel.x > 0 ?                     decompressRGB(voxel + ivec3(-1, 0, 0)) : ivec3(0);
    ivec3 dfXP = voxel.x < volumeDimension.x - 1 ? decompressRGB(voxel + ivec3(1, 0, 0)) : ivec3(0);
    ivec3 dfYN = voxel.y > 0 ?                     decompressRGB(voxel + ivec3(0, -1, 0)) : ivec3(0);
    ivec3 dfYP = voxel.y < volumeDimension.y - 1 ? decompressRGB(voxel + ivec3(0, 1, 0)) : ivec3(0);
    ivec3 dfZN = voxel.z > 0 ?                     decompressRGB(voxel + ivec3(0, 0, -1)) : ivec3(0);
    ivec3 dfZP = voxel.z < volumeDimension.z - 1 ? decompressRGB(voxel + ivec3(0, 0, 1)) : ivec3(0);

    ivec3 dfValue = maxv(dfXN, maxv(dfXP, maxv(dfYN, maxv(dfYP, maxv(dfZN, dfZP)))));

    dfValue -= ivec3(DECAY);
    dfValue = ivec3(max(0, dfValue.r), max(0, dfValue.g), max(0, dfValue.b));

    compressRGB(voxel, dfValue);
}