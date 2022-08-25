/**
 * Signed Distance Field.
 * Iterative calculation.
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, r16f) uniform image3D sdf;

uniform usampler3D tex_volume;
uniform ivec3 volumeDimension;
uniform ivec3 displacement;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#define FLT_MAX 1E+10

float getSDF(ivec3 voxel, bool solid) {
    float val = imageLoad(sdf, voxel).r;
    return solid ? min(1.0, val) : max(0.0, val);
}

void setSDF(ivec3 voxel, float value) {
    imageStore(sdf, voxel, vec4(value, 0, 0, 0));
}

bool isVoxelSolid(ivec3 voxel) {
    return texelFetch(tex_volume, voxel, 0).r > 0;
}

void main() {
    ivec3 voxel = ivec3(gl_GlobalInvocationID.xyz) + displacement;
    if (voxel.x >= volumeDimension.x || voxel.y >= volumeDimension.y || voxel.z >= volumeDimension.z) {
        return;
    }

    bool solid = isVoxelSolid(voxel);

    float sdfXN = voxel.x > 0 ? getSDF(voxel + ivec3(-1, 0, 0), solid) : FLT_MAX;
    float sdfXP = voxel.x < volumeDimension.x - 1 ? getSDF(voxel + ivec3(1, 0, 0), solid) : FLT_MAX;
    float sdfYN = voxel.y > 0 ? getSDF(voxel + ivec3(0, -1, 0), solid) : FLT_MAX;
    float sdfYP = voxel.y < volumeDimension.y - 1 ? getSDF(voxel + ivec3(0, 1, 0), solid) : FLT_MAX;
    float sdfZN = voxel.z > 0 ? getSDF(voxel + ivec3(0, 0, -1), solid) : FLT_MAX;
    float sdfZP = voxel.z < volumeDimension.z - 1 ? getSDF(voxel + ivec3(0, 0, 1), solid) : FLT_MAX;

    float sdf = solid ? max(sdfXN, max(sdfXP, max(sdfYN, max(sdfYP, max(sdfZN, sdfZP))))) - 1.0 : min(sdfXN, min(sdfXP, min(sdfYN, min(sdfYP, min(sdfZN, sdfZP))))) + 1.0;

    setSDF(voxel, sdf);
}