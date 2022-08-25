/**
 * Adjusted bilateral filter.
 * Reference: https://doi.org/10.1145/1281500.1281602
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, r32f) writeonly uniform image2D ambientOcclusionFilterTargetBuffer;

uniform isampler2D tex_voxelBuffer;
uniform isampler2D tex_normalBuffer;
uniform sampler2D tex_positionBuffer;
uniform sampler2D tex_ambientOcclusionBuffer;

uniform int kernelRadius;// kernelSize = 2 * kernelRadius + 1
uniform float sigmaSpatial;// for distance gaussian

layout(local_size_x = 16, local_size_y = 16) in;

#define PI 3.14159265359
#define sigmaRange 0.125

float gaussian1D(float val, float sigma) {
    return 1.0 / (2.0 * PI * sigma * sigma) * exp(- val * val / (2.0 * sigma * sigma));
}

ivec3 intToNormal(int n) {
    return ((1 & n) > 0 ? -1 : 1) * ivec3(sign(2 & n), sign(4 & n), sign(8 & n));
}

void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(ambientOcclusionFilterTargetBuffer);
    if (pixel.x >= size.x || pixel.y >= size.y) {
        return;
    }
    if (texelFetch(tex_positionBuffer, pixel, 0).a == 0.0) {
        imageStore(ambientOcclusionFilterTargetBuffer, pixel, vec4(1.0, 0, 0, 0));// invalid position
        return;
    }

    float ao = texelFetch(tex_ambientOcclusionBuffer, pixel, 0).r;
    ivec3 voxel = texelFetch(tex_voxelBuffer, pixel, 0).xyz;
    vec3 N = intToNormal(texelFetch(tex_normalBuffer, ivec2(pixel), 0).r);

    float normalization = 0.0;
    float val = 0.0;

    for (int dy = -kernelRadius; dy <= kernelRadius; dy++) {
        for (int dx = -kernelRadius; dx <= kernelRadius; dx++) {
            vec3 _N = intToNormal(texelFetch(tex_normalBuffer, ivec2(pixel) + ivec2(dx, dy), 0).r);
            float _ao = texelFetch(tex_ambientOcclusionBuffer, pixel + ivec2(dx, dy), 0).r;
            ivec3 _voxel = texelFetch(tex_voxelBuffer, pixel + ivec2(dx, dy), 0).xyz;
//                        float factor = gaussian1D(sqrt(dx * dx + dy * dy), sigmaSpatial); // gaussian blur
//                        float factor = gaussian1D(sqrt(dx * dx + dy * dy), sigmaSpatial) * gaussian1D(abs(_ao - ao), sigmaRange); // default biliteral filter
            float factor = gaussian1D(sqrt(dx * dx + dy * dy), sigmaSpatial) * (abs(dot(voxel, N) - dot(_voxel, _N)) == 0 ? 1 : 0) * max(0, dot(N, _N));// our bilateral filter
            val += factor * _ao;
            normalization += factor;
        }
    }
    val /= normalization;

    imageStore(ambientOcclusionFilterTargetBuffer, pixel, vec4(val, 0, 0, 0));
}