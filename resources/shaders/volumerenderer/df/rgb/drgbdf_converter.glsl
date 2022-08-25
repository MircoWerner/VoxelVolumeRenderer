/**
 * Third step of the Directional RGB Distance Field calculation.
 * Stores the maximum light level that reaches a voxel.
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, r16ui) uniform uimage3D df;// [ 1bit | 5bit blue | 5bit green | 5bit red ]

uniform usampler3D tex_directional_rgb_df;// rgba32ui, single color

uniform usampler3D tex_volume;

uniform ivec3 rgbDimension;// dimension of the rgb sdf buffer
uniform ivec3 displacement;// displacement in the rgb sdf buffer

uniform int rgbIdx;

uniform int zDisplacement;// displacement in the volume

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#define RED_MASK uint(0x001F)
#define GREEN_MASK uint(0x03E0)
#define BLUE_MASK uint(0x7C00)
#define ALPHA_MASK uint(0x8000)

#define GREEN_SHIFT 5
#define BLUE_SHIFT 10
#define ALPHA_SHIFT 15

uvec3 decompressRGB(ivec3 voxel) {
    uint value = imageLoad(df, voxel).r;
    uvec3 rgb;
    rgb.r = value & RED_MASK;
    rgb.g = (value & GREEN_MASK) >> GREEN_SHIFT;
    rgb.b = (value & BLUE_MASK) >> BLUE_SHIFT;
    return rgb;
}

void compressRGB(ivec3 voxel, uvec3 rgb) {
    uint value = (rgb.b << BLUE_SHIFT) | (rgb.g << GREEN_SHIFT) | (rgb.r);
    imageStore(df, voxel, uvec4(value, 0, 0, 0));
}

uint decodeMaximum(uvec4 dfValue) {
    uint maxValue = 0;
    for (int componentIdx = 0; componentIdx < 4; componentIdx++) {
        uint component = dfValue[componentIdx];
        for (int innerIdx = 0; innerIdx < 8; innerIdx++) {
            uint value = component >> (innerIdx * 4);// shift relevant part to lower 4 bits
            value = value & uint(0xF);// masking
            maxValue = max(maxValue, value);
        }
    }
    return maxValue;
}

void main() {
    ivec3 voxel = ivec3(gl_GlobalInvocationID.xyz) + displacement;
    if (voxel.x >= rgbDimension.x || voxel.y >= rgbDimension.y || voxel.z >= rgbDimension.z) {
        return;
    }

    uint maxValue = decodeMaximum(texelFetch(tex_directional_rgb_df, voxel, 0));

    uvec3 dfValue = decompressRGB(voxel + ivec3(0, 0, zDisplacement));
    dfValue[rgbIdx] = max(dfValue[rgbIdx], maxValue);
    compressRGB(voxel + ivec3(0, 0, zDisplacement), dfValue);
}