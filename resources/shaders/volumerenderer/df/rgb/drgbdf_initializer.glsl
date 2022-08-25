/**
 * First step of the Directional RGB Distance Field calculation.
 * Initializes light emitting voxels.
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, rgba32ui) writeonly uniform uimage3D df;

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

uniform ivec3 rgbDimension; // dimension of the rgb df buffer
uniform ivec3 displacement; // displacement in the rgb df buffer

uniform int rgbIdx;

uniform int zDisplacement; // displacement in the volume

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

uvec4 encodeValue(uint value) {
    uvec4 dfValue = uvec4(0);
    for (int componentIdx = 0; componentIdx < 4; componentIdx++) {
        uint component = 0;
        for (int innerIdx = 0; innerIdx < 8; innerIdx++) {
            component = component | (value << (4 * innerIdx));
        }
        dfValue[componentIdx] = component;
    }
    return dfValue;
}

void main() {
    ivec3 voxel = ivec3(gl_GlobalInvocationID.xyz) + displacement;
    if (voxel.x >= rgbDimension.x || voxel.y >= rgbDimension.y || voxel.z >= rgbDimension.z) {
        return;
    }

    uint tex = texelFetch(tex_volume, voxel + ivec3(0, 0, zDisplacement), 0).r;

    if (tex == 0) { // not solid
        imageStore(df, voxel, uvec4(0));
        return;
    }

    uvec3 color = uvec3(cellProperties[tex].emittance *  vec3(cellProperties[tex].red, cellProperties[tex].green, cellProperties[tex].blue) * 15);
    uint value = color[rgbIdx];

    uvec4 encoded = encodeValue(value);
    imageStore(df, voxel, encoded);
}