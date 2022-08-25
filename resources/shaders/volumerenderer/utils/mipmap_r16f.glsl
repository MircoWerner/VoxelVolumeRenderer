/**
 * Hardware accelerated mip map creation.
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, r16f) writeonly uniform image3D outputTexture;

uniform ivec3 outputDimension;

uniform sampler3D inputTexture;
uniform ivec3 inputDimension;
uniform int inputLevel;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

void main() {
    ivec3 texel = ivec3(gl_GlobalInvocationID.xyz);
    if (texel.x >= outputDimension.x || texel.y >= outputDimension.y || texel.z >= outputDimension.z) {
        return;
    }

    ivec3 lookupTexel = 2 * texel;

    vec3 INV_TEXTURE = 1.0 / inputDimension;
    // values are clamped, no edge case detection necessary
    float val000 = textureLod(inputTexture, (lookupTexel + ivec3(0, 0, 0) + vec3(0.5)) * INV_TEXTURE, inputLevel).r;
    float val001 = textureLod(inputTexture, (lookupTexel + ivec3(0, 0, 1) + vec3(0.5)) * INV_TEXTURE, inputLevel).r;
    float val010 = textureLod(inputTexture, (lookupTexel + ivec3(0, 1, 0) + vec3(0.5)) * INV_TEXTURE, inputLevel).r;
    float val011 = textureLod(inputTexture, (lookupTexel + ivec3(0, 1, 1) + vec3(0.5)) * INV_TEXTURE, inputLevel).r;
    float val100 = textureLod(inputTexture, (lookupTexel + ivec3(1, 0, 0) + vec3(0.5)) * INV_TEXTURE, inputLevel).r;
    float val101 = textureLod(inputTexture, (lookupTexel + ivec3(1, 0, 1) + vec3(0.5)) * INV_TEXTURE, inputLevel).r;
    float val110 = textureLod(inputTexture, (lookupTexel + ivec3(1, 1, 0) + vec3(0.5)) * INV_TEXTURE, inputLevel).r;
    float val111 = textureLod(inputTexture, (lookupTexel + ivec3(1, 1, 1) + vec3(0.5)) * INV_TEXTURE, inputLevel).r;

    // average
    float val = (val000 + val001 + val010 + val011 + val100 + val101 + val110 + val111) / 8.0;

    imageStore(outputTexture, texel, vec4(val, 0, 0, 0));
}
