/**
 * First step of the RGB Distance Field calculation.
 * Initializes light emitting voxels.
 * Reference: https://0fps.net/2018/02/21/voxel-lighting/
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, r16ui) writeonly uniform uimage3D df;// [ 1bit | 5bit blue | 5bit green | 5bit red ]

struct CellProperties {
    float red;
    float green;
    float blue;
    float alpha;
    float emittance;
    float roughness;
};
layout(binding = 1, std430) buffer cellPropertiesBuffer {
    CellProperties cellProperties[];
};

uniform usampler3D tex_volume;
uniform ivec3 volumeDimension;
uniform ivec3 displacement;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#define RED_MASK 0x001F
#define GREEN_MASK 0x03E0
#define BLUE_MASK 0x7C00

#define GREEN_SHIFT 5
#define BLUE_SHIFT 10

void compressRGB(ivec3 voxel, uvec3 rgb) {
    uint value = (rgb.b << BLUE_SHIFT) | (rgb.g << GREEN_SHIFT) | (rgb.r);
    imageStore(df, voxel, uvec4(value, 0, 0, 0));
}

void main() {
    ivec3 voxel = ivec3(gl_GlobalInvocationID.xyz) + displacement;
    if (voxel.x >= volumeDimension.x || voxel.y >= volumeDimension.y || voxel.z >= volumeDimension.z) {
        return;
    }

    uint tex = texelFetch(tex_volume, voxel, 0).r;

    if (tex == 0) { // not solid
        imageStore(df, voxel, uvec4(0));
        return;
    }

    uvec3 color = uvec3(cellProperties[tex].emittance * vec3(cellProperties[tex].red, cellProperties[tex].green, cellProperties[tex].blue) * 15);
    compressRGB(voxel, color);
}