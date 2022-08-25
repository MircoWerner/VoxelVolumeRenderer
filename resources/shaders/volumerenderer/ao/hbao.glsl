/**
 * Horizon-Based Ambient Occlusion.
 * Reference: https://doi.org/10.1145/1401032.1401061
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, r32f) uniform image2D ambientOcclusionBuffer;

uniform sampler2D tex_depthBuffer;
uniform isampler2D tex_normalBuffer;
uniform sampler2D tex_positionBuffer;// [x,y,z,valid], valid == 1 if encodes hit, 0 otherwise (background)

uniform mat4 viewMatrix;// world space -> view space
uniform mat4 inverseProjection;// image/screen space -> view space

uniform int Nd;// number of directions phi
uniform int Ns;// number of setps in each direction phi
uniform float stepSize;// step size (step size in view-space)
uniform float R;// radius of influence (distance in view-space)
uniform float tangentBias;// tangent angle bias (optional)

layout(local_size_x = 16, local_size_y = 16) in;

#define PI 3.14159265359

// http://www.jcgt.org/published/0009/03/02/
uvec3 pcg3d(uvec3 v) {
    v = v * 1664525u + 1013904223u;
    v.x += v.y*v.z;
    v.y += v.z*v.x;
    v.z += v.x*v.y;
    v ^= v >> 16u;
    v.x += v.y*v.z;
    v.y += v.z*v.x;
    v.z += v.x*v.y;
    return v;
}

ivec3 intToNormal(int n) {
    return ((1 & n) > 0 ? -1 : 1) * ivec3(sign(2 & n), sign(4 & n), sign(8 & n));
}

// https://github.com/nvpro-samples/gl_ssao/blob/master/hbao.frag.glsl#L186
vec2 rotateDirection(vec2 dir, vec2 cosSin) {
    return vec2(dir.x * cosSin.x - dir.y * cosSin.y, dir.x * cosSin.y + dir.y * cosSin.x);
}

vec3 iscreenSpaceToViewSpace(vec2 uv, ivec2 pixel) {
    vec3 screenSpace = vec3(uv, texelFetch(tex_depthBuffer, pixel, 0).r);
    vec3 ndc = (2.0 * screenSpace) - 1.0;// normalized device coordinates
    vec4 unprojected = inverseProjection * vec4(ndc, 1.0);
    return unprojected.xyz / unprojected.w;// view-space
}

vec3 screenSpaceToViewSpace(vec2 uv) {
    vec3 screenSpace = vec3(uv, textureLod(tex_depthBuffer, uv, 0).r);
    vec3 ndc = 2.0 * screenSpace - 1.0;// normalized device coordinates
    vec4 unprojected = inverseProjection * vec4(ndc, 1.0);
    return unprojected.xyz / unprojected.w;// view-space
}

void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(ambientOcclusionBuffer);
    if (pixel.x >= size.x || pixel.y >= size.y) {
        return;
    }
    vec4 position = texelFetch(tex_positionBuffer, pixel, 0);
    if (position.a == 0.0) {
        imageStore(ambientOcclusionBuffer, pixel, vec4(1, 0, 0, 0));
        return;
    }

    vec2 uv = vec2(pixel) / vec2(size);

    // position
    vec3 P = iscreenSpaceToViewSpace(uv, pixel);// position (view-space)

    // normal
    vec3 N_worldSpace = intToNormal(texelFetch(tex_normalBuffer, ivec2(pixel), 0).r);
    vec3 N = normalize((viewMatrix * vec4(N_worldSpace, 0)).xyz);// normal (view-space)

    // step size
    float step = stepSize / -P.z;// step size (image-space)

    // random
    vec3 random = vec3(pcg3d(uvec3(position.xyz * 100))) * (1.0/float(0xffffffffu));

    float sum = 0.0;
    float uniformDirectionAngleFactor = 2.0 * PI / Nd;
    for (int d = 0; d < Nd; d++) {
        float th = d * uniformDirectionAngleFactor;// uniform direction angle
        vec2 phi = normalize(rotateDirection(vec2(cos(th), sin(th)), random.xy));// randomized direction

        float tangentAngle = acos(dot(vec3(phi, 0.0), N)) - 0.5 * PI + tangentBias;
        float horizonAngle = tangentAngle;// horizon angle = max(tangentAngle, elevationAngle_i i=0..Ns-1)
        float horizonDistance = 0.0;// distance between P and the horizon point

        for (int i = 0; i < Ns; i++) {
            vec2 stepPosition = uv + (random.z + i + 1.0) * step * phi;

            vec2 samplePixel = stepPosition * size;
            ivec2 snappedSamplePixel = ivec2(round(samplePixel.x), round(samplePixel.y));// snap to texel centers
            vec3 Si = iscreenSpaceToViewSpace((snappedSamplePixel + vec2(0.5)) / size, snappedSamplePixel);// sampled position (view-space)

            vec3 D = Si - P;

            float distance = length(D);
            if (distance > R) {
                continue;// ignore samples that are outside of the radius of influence
            }

            float elevationAngle = atan(D.z / length(D.xy));// elevation angle

            horizonDistance = elevationAngle > horizonAngle ? distance : horizonDistance;// update distance if new horizon point is found
            horizonAngle = max(horizonAngle, elevationAngle);// update horizon angle if new horizon point is found
        }

        float attenuation = 1.0 - horizonDistance / float(R);// linear attenuation factor
        sum += 1.0 - 2.0 * clamp((sin(horizonAngle) - sin(tangentAngle)) * attenuation, 0.0, 1.0);
    }

    sum /= Nd;

    imageStore(ambientOcclusionBuffer, pixel, vec4(sum, 0, 0, 0));
}