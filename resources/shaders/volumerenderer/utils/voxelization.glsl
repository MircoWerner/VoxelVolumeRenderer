/**
 * "Voxelization".
 * Our scene is already voxelized. This shader just stores the local occlusion values and the emission values in the textures.
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, rgba8) uniform image3D albedo;//rgb light at emissive sources, reduced resolution, alpha component is ignored
layout(binding = 1, r8) uniform image3D alpha;// local occlusion values

struct CellProperties {
    float red;
    float green;
    float blue;
    float alpha;
    float emittance;
    float roughness;
};
layout(binding = 2, std430) buffer cellPropertiesBuffer {
    CellProperties cellProperties[];
};

uniform usampler3D tex_volume;
uniform ivec3 volumeDimension;
uniform ivec3 compressedDimension;
uniform ivec3 displacement;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

bool isVoxelSolid(ivec3 voxel) {
    return texelFetch(tex_volume, voxel, 0).r > 0;
}

vec3 getColor(ivec3 voxel) {
    uint tex = texelFetch(tex_volume, voxel, 0).r;
    return tex == 0 ? vec3(0) : cellProperties[tex].emittance * vec3(cellProperties[tex].red, cellProperties[tex].green, cellProperties[tex].blue);
}

void compressRGBA(ivec3 voxel) {
    if (isVoxelSolid(voxel)) {
        imageStore(alpha, voxel, vec4(1.0, 0.0, 0.0, 0.0));
    } else {
        imageStore(alpha, voxel, vec4(0.0, 0.0, 0.0, 0.0));
    }

    if (voxel.x >= compressedDimension.x || voxel.y >= compressedDimension.y || voxel.z >= compressedDimension.z) {
        return;
    }

    ivec3 lookupVoxel = 2 * voxel;

    // values are clamped, no edge case detection necessary
    vec3 val000 = getColor(lookupVoxel + ivec3(0, 0, 0));
    vec3 val001 = getColor(lookupVoxel + ivec3(0, 0, 1));
    vec3 val010 = getColor(lookupVoxel + ivec3(0, 1, 0));
    vec3 val011 = getColor(lookupVoxel + ivec3(0, 1, 1));
    vec3 val100 = getColor(lookupVoxel + ivec3(1, 0, 0));
    vec3 val101 = getColor(lookupVoxel + ivec3(1, 0, 1));
    vec3 val110 = getColor(lookupVoxel + ivec3(1, 1, 0));
    vec3 val111 = getColor(lookupVoxel + ivec3(1, 1, 1));

    // average
    vec3 val = (val000 + val001 + val010 + val011 + val100 + val101 + val110 + val111) / 8.0;

    imageStore(albedo, voxel, vec4(val, 1));
}

void main() {
    ivec3 voxel = ivec3(gl_GlobalInvocationID.xyz) + displacement;
    if (voxel.x >= volumeDimension.x || voxel.y >= volumeDimension.y || voxel.z >= volumeDimension.z) {
        return;
    }

    compressRGBA(voxel);
}