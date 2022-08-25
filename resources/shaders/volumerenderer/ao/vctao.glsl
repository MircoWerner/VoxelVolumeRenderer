/**
 * Voxel Cone Traced Ambient Occlusion.
 * Reference: https://doi.org/10.1111/j.1467-8659.2011.02063.x
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, r32f) uniform image2D ambientOcclusionBuffer;

uniform sampler2D tex_positionBuffer;// [x,y,z,valid], valid == 1 if encodes hit, 0 otherwise (background)
uniform isampler2D tex_normalBuffer;

uniform ivec3 volumeDimension;

uniform sampler3D tex_alpha;// local occlusion values

uniform int coneTraceSteps;
uniform float coneTraceStepSize;
uniform float coneApertureAngle;// radians
uniform float attenuation;

layout(local_size_x = 16, local_size_y = 16) in;

#define PI 3.14159265359

ivec3 intToNormal(int n) {
    return ((1 & n) > 0 ? -1 : 1) * ivec3(sign(2 & n), sign(4 & n), sign(8 & n));
}

vec3 createDirection(float phi, float theta, vec3 N) {
    float x = sin(theta) * cos(phi);
    float y = sin(theta) * sin(phi);
    float z = cos(theta);

    if (N.x != 0) {
        return vec3(sign(N.x) * z, x, y);
    } else if (N.y != 0) {
        return vec3(x, sign(N.y) * z, y);
    } else {
        return vec3(x, y, sign(N.z) * z);
    }
}

float attenuate(float distance) {
    return 1.0 / (1.0 + distance / attenuation);
}

float coneTraceAlpha(vec3 P, vec3 N, vec3 direction, int coneTraceSteps, float aperture) {
    vec3 INV_TEXTURE = 1.0 / volumeDimension;

    float alpha = 0.0;
    float tanHalfAperture = tan(aperture / 2.0);

    float distance = coneTraceStepSize;
    float distanceIncrease = distance;
    for (int i = 0; i < coneTraceSteps && alpha <= 0.99; i++) {
        vec3 position = P + distance * direction;
        if (position.x < 0 || position.x > volumeDimension.x || position.y < 0 || position.y > volumeDimension.y || position.z < 0 && position.z > volumeDimension.z) {
            break;
        }
        float coneDiameter = max(1.0, 2.0 * tanHalfAperture * distance);
        float alphaStep = textureLod(tex_alpha, position * INV_TEXTURE, log2(coneDiameter)).r;// alpha lookup at mip map level
        alpha += attenuate(distance) * (1.0 - alpha) * (1 - pow(1 - alphaStep, distanceIncrease / coneDiameter));// front to back accumulation, add attenuation, correct alpha value
        distanceIncrease = (1.0 + coneDiameter) * coneTraceStepSize;
        distance += distanceIncrease;// increase distance
    }
    return alpha;
}

float ambientOcclusion(vec3 P, vec3 N) {
    float alpha = 0.0;
    alpha += coneTraceAlpha(P, N, N, coneTraceSteps, coneApertureAngle);
    alpha += coneTraceAlpha(P, N, createDirection(0.785398, 0.785398, N), coneTraceSteps, coneApertureAngle);
    alpha += coneTraceAlpha(P, N, createDirection(2.35619, 0.785398, N), coneTraceSteps, coneApertureAngle);
    alpha += coneTraceAlpha(P, N, createDirection(3.92699, 0.785398, N), coneTraceSteps, coneApertureAngle);
    alpha += coneTraceAlpha(P, N, createDirection(5.49779, 0.785398, N), coneTraceSteps, coneApertureAngle);
    return 1.0 - alpha / 5.0;
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

    float ao = ambientOcclusion(position.xyz, normal);

    imageStore(ambientOcclusionBuffer, pixel, vec4(ao, 0, 0, 0));
}
