/**
 * Ray Traced Ambient Occlusion.
 * Reference: https://doi.org/10.1201/b22086
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, r32f) uniform image2D ambientOcclusionBuffer;

uniform sampler2D tex_positionBuffer;// [x,y,z,valid], valid == 1 if encodes hit, 0 otherwise (background)
uniform isampler2D tex_normalBuffer;

uniform usampler3D tex_volume;
uniform sampler3D tex_sdf;// city block distance
uniform ivec3 volumeDimension;

uniform int samples;// number of samples in this execution
uniform float distanceToHalfOcclusion;

uniform int accumulatedSamples;// total number of accumulated samples after this execution

layout(local_size_x = 16, local_size_y = 16) in;

#define FLT_MAX 1E+10
#define EPSILON 0.0001
#define PI 3.14159265359
#define MAX_ITERATIONS 1000

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

vec3 random(ivec2 pixel) {
    return vec3(pcg3d(uvec3(pixel, uint(pixel.x) ^ uint(pixel.y)))) * (1.0/float(0xffffffffu));
}

ivec3 intToNormal(int n) {
    return ((1 & n) > 0 ? -1 : 1) * ivec3(sign(2 & n), sign(4 & n), sign(8 & n));
}

vec3 uniformSampleSphere(float u1, float u2) {
    float h = 1.0 - 2.0 * u1;
    float r = sqrt(1.0 - h * h);
    return vec3(r * cos(2.0 * PI * u2), h, r * sin(2.0 * PI * u2));
}

vec3 uniformSampleHemisphere(float u1, float u2, vec3 N) {
    vec3 dir = uniformSampleSphere(u1, u2);
    if (dot(N, dir) < 0.0) {
        dir = -dir;
    }
    return dir;
}

vec3 min3V(vec3 v, vec3 w) {
    return vec3(min(v.x, w.x), min(v.y, w.y), min(v.z, w.z));
}

vec3 max3V(vec3 v, vec3 w) {
    return vec3(max(v.x, w.x), max(v.y, w.y), max(v.z, w.z));
}

bool intersectAABB(vec3 minAABB, vec3 maxAABB, vec3 origin, vec3 direction, inout float tMin, inout float tMax) {
    vec3 div = vec3(1.0, 1.0, 1.0) / direction;
    vec3 t1 = (minAABB - origin) * div;
    vec3 t2 = (maxAABB - origin) * div;

    vec3 tMin2 = min3V(t1, t2);
    vec3 tMax2 = max3V(t1, t2);

    tMin = max(max(tMin2.x, tMin2.y), max(tMin2.z, tMin));
    tMax = min(min(tMax2.x, tMax2.y), min(tMax2.z, tMax));

    return tMin <= tMax;
}

// https://www.researchgate.net/publication/2611491_A_Fast_Voxel_Traversal_Algorithm_for_Ray_Tracing
// https://doi.org/10.1007/BF01900697
bool sdfRayMarching(vec3 position, vec3 direction, out vec3 intersection) {
    vec3 e = position;

    vec3 pos = e;
    vec3 d = direction;
    ivec3 step = ivec3(sign(d.x), sign(d.y), sign(d.z));

    // current cell index
    ivec3 cell = ivec3(floor(pos.x), floor(pos.y), floor(pos.z));
    // how much t increases when moving through one cell
    vec3 deltaT = vec3(d.x == 0 ? FLT_MAX : (sign(d.x) / d.x), d.y == 0 ? FLT_MAX : (sign(d.y) / d.y), d.z == 0 ? FLT_MAX : (sign(d.z) / d.z));
    // calculate t values such that e+t.i*d is the minimal value that the ray intersects with the next cell on the i-axis
    vec3 nextT = vec3(d.x == 0 ? FLT_MAX : ((floor(e.x) + (sign(d.x) >= 0.0 ? 1 : 0) - e.x) / d.x),
    d.y == 0 ? FLT_MAX : ((floor(e.y) + (sign(d.y) >= 0.0 ? 1 : 0) - e.y) / d.y),
    d.z == 0 ? FLT_MAX : ((floor(e.z) + (sign(d.z) >= 0.0 ? 1 : 0) - e.z) / d.z));

    int iteration = 0;
    do {
        uint tex = texelFetch(tex_volume, cell, 0).r;
        if (tex > 0) {
            intersection = pos;
            return true;
        }

        float sdf = texelFetch(tex_sdf, cell, 0).r - 3;
        if (sdf <= 0) {
            // DDA
            float tMin = min(nextT.x, min(nextT.y, nextT.z));
            ivec3 axis = ivec3(nextT.x == tMin, nextT.y == tMin, nextT.z == tMin);
            nextT.xyz += deltaT.xyz * axis;
            cell += step * axis;

            pos = e + tMin * d;
        } else {
            // fast traversal
            vec3 C = sdf / (abs(d.x) + abs(d.y) + abs(d.z)) * d;
            pos.xyz += C;

            cell = ivec3(floor(pos.x), floor(pos.y), floor(pos.z));

            nextT.xyz = vec3(d.x == 0 ? FLT_MAX : ((floor(pos.x) + (sign(d.x) >= 0.0 ? 1 : 0) - e.x) / d.x),
            d.y == 0 ? FLT_MAX : ((floor(pos.y) + (sign(d.y) >= 0.0 ? 1 : 0) - e.y) / d.y),
            d.z == 0 ? FLT_MAX : ((floor(pos.z) + (sign(d.z) >= 0.0 ? 1 : 0) - e.z) / d.z));
        }

        iteration++;
    } while (-1 <= cell.x && cell.x <= volumeDimension.x && -1 <= cell.y && cell.y <= volumeDimension.y && -1 <= cell.z && cell.z <= volumeDimension.z && iteration < MAX_ITERATIONS);

    return false;
}

float maxUnobstructedDistance(vec3 P, vec3 dir) {
    vec3 isect;
    if (sdfRayMarching(P, dir, isect)) {
        return length(isect - P);
    }
    return FLT_MAX;
}

float evaluateRayTracedAmbientOcclusion(vec3 P, vec3 N, ivec2 pixel) {
    float ambientOcclusion = 0.0;
    for (int i = 1; i <= samples; i++) {
        vec3 rdm = random(i * pixel);
        vec3 dir = uniformSampleHemisphere(rdm.x, rdm.y, N);
        float distance = maxUnobstructedDistance(P + EPSILON * N, dir);
        float V = (distance < FLT_MAX) ? (1.0 - 1.0 / (1.0 + (distance * distance) / (distanceToHalfOcclusion * distanceToHalfOcclusion))) : 1.0;
        float cosTheta = max(0.0, dot(N, dir));
        ambientOcclusion += V * cosTheta;
    }
    ambientOcclusion *= 2.0;
    return ambientOcclusion;
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

    float aoOld = imageLoad(ambientOcclusionBuffer, pixel).r;
    ivec3 seed = ivec3(position.xyz * accumulatedSamples);
    float ao = evaluateRayTracedAmbientOcclusion(position.xyz, normal, pixel + ivec2(seed.x ^ seed.y, seed.y ^ seed.z));

    float aoNew = (aoOld * (accumulatedSamples - samples) + ao) / float(accumulatedSamples);// accumulation

    imageStore(ambientOcclusionBuffer, pixel, vec4(aoNew, 0, 0, 0));
}
