/**
 * Distance Field Ambient Occlusion.
 * Reference: https://doi.org/10.1145/1185657.1185834
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, r32f) uniform image2D ambientOcclusionBuffer;

uniform sampler2D tex_positionBuffer;// [x,y,z,valid], valid == 1 if encodes hit, 0 otherwise (background)
uniform isampler2D tex_normalBuffer;

uniform sampler3D tex_sdf;// city block
uniform ivec3 volumeDimension;

uniform int numberOfSteps;
uniform float contrast;

layout(local_size_x = 16, local_size_y = 16) in;

ivec3 intToNormal(int n) {
    return ((1 & n) > 0 ? -1 : 1) * ivec3(sign(2 & n), sign(4 & n), sign(8 & n));
}

float sdfTextureLookup(vec3 coordinate, float lod) {
    return textureLod(tex_sdf, coordinate, lod).r;
}

float evaluateDFAmbientOcclusion(vec3 P, vec3 N) {
    vec3 inv = vec3(1.0 / volumeDimension.x, 1.0 / volumeDimension.y, 1.0 / volumeDimension.z);

    float sum = 0.0;
    int di = 1;
    for (int i = 0; i < numberOfSteps; i++) {
        sum += (sdfTextureLookup((P + di * N - 0.5 * N) * inv, i) - di);
        di *= 2;
    }
    sum = exp(contrast * sum);
    return clamp(sum, 0, 1);
}

void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(ambientOcclusionBuffer);
    if (pixel.x >= size.x || pixel.y >= size.y) {
        return;
    }

    vec4 position = texelFetch(tex_positionBuffer, pixel, 0);
    if (position.a == 0) {
        imageStore(ambientOcclusionBuffer, pixel, vec4(1, 0, 0, 0));
        return;
    }
    ivec3 normal = intToNormal(texelFetch(tex_normalBuffer, pixel, 0).r);

    float ao = evaluateDFAmbientOcclusion(position.xyz, normal);

    imageStore(ambientOcclusionBuffer, pixel, vec4(ao, 0, 0, 0));
}
