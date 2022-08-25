/**
 * Accelerated Ray Marching.
 * Renders the screen space scene information to the buffers.
 * References: https://www.researchgate.net/publication/2611491_A_Fast_Voxel_Traversal_Algorithm_for_Ray_Tracing, https://doi.org/10.1007/BF01900697
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, rgba32f) uniform image2D positionBuffer;// [x,y,z,valid], valid == 1 if encodes hit, 0 otherwise (background)
layout(binding = 1, rgba32f) uniform image2D albedoBuffer;
layout(binding = 2, r32f) uniform image2D depthBuffer;
layout(binding = 3, r8i) uniform iimage2D normalBuffer;
layout(binding = 4, rgba16i) uniform iimage2D voxelBuffer;

struct CellProperties {
    float red;
    float green;
    float blue;
    float alpha;
    float emittance;
    float roughness;
};
layout(binding = 6, std430) buffer cellPropertiesBuffer {
    CellProperties cellProperties[];
};

uniform usampler3D tex_volume;
uniform sampler3D tex_sdf;// city block distance
uniform ivec3 volumeDimension;

uniform vec3 ray00;
uniform vec3 ray01;
uniform vec3 ray10;
uniform vec3 ray11;
uniform vec3 rayOrigin;

uniform int udbgSdf;
uniform int udbgSdfValue;

uniform mat4 MVP;// world coordinates -> projection coordinates

layout(local_size_x = 16, local_size_y = 16) in;

#define FLT_MAX 1E+10
#define EPSILON 0.0001
#define PI 3.14159265359
#define MAX_ITERATIONS 10000
//#define BACKGROUND vec3(0.529, 0.808, 0.922) * vec3(0.5)
#define BACKGROUND vec3(1)

/*
    === STRUCTS and SCENE INFORMATION ===
*/
struct Light {
    vec3 P;// position
    vec3 I;// intensity (r,g,b)
};

struct Intersection {
    float t;
    vec3 P;
    ivec3 N;
    ivec3 voxel;
    vec3 albedo;
};

const Light light = { vec3(128, 128, -100), vec3(1, 1, 1) };
const Light light2 = { vec3(128, 128, 500), vec3(1, 1, 1) };

vec3 evaluatePhong(vec3 P, vec3 N, vec3 V, Light light, vec3 k_d) {
    vec3 L = normalize(light.P - P);
    vec3 R = normalize(reflect(-L, N));
    return light.I * k_d * max(0.2, dot(N, L));
}

vec3 evaluateFinalColor(vec3 P, vec3 N, vec3 k_d) {
    return evaluatePhong(P, N, normalize(rayOrigin - P), light, k_d) + evaluatePhong(P, N, normalize(rayOrigin - P), light2, k_d);
}

/*
    === UTILITY METHODS ===
*/
vec3 min3V(vec3 v, vec3 w) {
    return vec3(min(v.x, w.x), min(v.y, w.y), min(v.z, w.z));
}

vec3 max3V(vec3 v, vec3 w) {
    return vec3(max(v.x, w.x), max(v.y, w.y), max(v.z, w.z));
}

/*
    === DEFERRED RENDERING BUFFERS ===
*/
int normalToInt(ivec3 normal) {
    int signBit = (sign(normal.x) + sign(normal.y) + sign(normal.z)) < 0 ? 1 : 0;
    return (abs(sign(normal.z)) << 3) | (abs(sign(normal.y)) << 2) | (abs(sign(normal.x)) << 1) | signBit;
}

ivec3 intToNormal(int n) {
    return ((1 & n) > 0 ? -1 : 1) * ivec3(sign(2 & n), sign(4 & n), sign(8 & n));
}

void writeBuffers(ivec2 pixel, vec4 position, ivec3 normal, vec3 color, float depth, ivec3 voxel) {
    imageStore(positionBuffer, pixel, position);
    imageStore(albedoBuffer, pixel, vec4(color, 0));
    imageStore(depthBuffer, pixel, vec4(depth, 0, 0, 0));
    imageStore(normalBuffer, pixel, ivec4(normalToInt(normal), 0, 0, 0));
    imageStore(voxelBuffer, pixel, ivec4(voxel, 0));
}

float calcDepth(vec3 position) {
    vec4 projVec = (MVP * vec4(position, 1));
    return projVec.z / projVec.w;
}

/*
    === RAY MARCHING ===
*/
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

ivec3 calcNormal(vec3 d, ivec3 axis) {
    return -ivec3(sign(d.x), sign(d.y), sign(d.z)) * axis;
}

// slow, for debug purposes of the sdf only
bool rayMarching(vec3 position, vec3 direction, out Intersection intersection) {
    vec3 INV_TEXTURE = 1.0 / volumeDimension;

    vec3 e = position;
    {
        // intersect with bounding volume
        float tMin = 0.0;
        float tMax = FLT_MAX;
        if (!intersectAABB(vec3(-1, -1, -1), volumeDimension + vec3(1, 1, 1), position, direction, tMin, tMax)) {
            return false;
        }

        // move start position of ray to intersection with bounding volume
        e = position + tMin * direction;
    }
    vec3 pos = e;// current position
    vec3 d = direction;
    ivec3 step = ivec3(sign(d.x), sign(d.y), sign(d.z));

    // current cell index
    ivec3 cell = ivec3(floor(pos.x), floor(pos.y), floor(pos.z));
    // how much t increases when moving through one cell along each axis
    vec3 deltaT = vec3(d.x == 0 ? FLT_MAX : (sign(d.x) / d.x), d.y == 0 ? FLT_MAX : (sign(d.y) / d.y), d.z == 0 ? FLT_MAX : (sign(d.z) / d.z));
    // e+nextT.i*d is the minimal value, s.t. the ray intersects with the next cell on the i-axis
    vec3 nextT = vec3(d.x == 0 ? FLT_MAX : ((floor(e.x) + (sign(d.x) >= 0.0 ? 1 : 0) - e.x) / d.x),
    d.y == 0 ? FLT_MAX : ((floor(e.y) + (sign(d.y) >= 0.0 ? 1 : 0) - e.y) / d.y),
    d.z == 0 ? FLT_MAX : ((floor(e.z) + (sign(d.z) >= 0.0 ? 1 : 0) - e.z) / d.z));

    // initialize axis for lighting calculation of cells along the outside
    ivec3 axis = ivec3(0, 0, 0);
    if (nextT.x <= nextT.y && nextT.x <= nextT.z) {
        axis = ivec3(1, 0, 0);
    } else if (nextT.y <= nextT.x && nextT.y <= nextT.z) {
        axis = ivec3(0, 1, 0);
    } else {
        axis = ivec3(0, 0, 1);
    }

    int iteration = 0;
    do {
        uint tex = texelFetch(tex_volume, cell, 0).r;// get volume information of current cell
        float sdf = textureLod(tex_sdf, (cell + vec3(0.5)) * INV_TEXTURE, 0).r;// distance field value, for debug
        if (sdf >= udbgSdfValue && sdf < udbgSdfValue + 1.f && cell.x >= 0 && cell.y >= 0 && cell.z >= 0 && cell.x < volumeDimension.x && cell.y < volumeDimension.y && cell.z < volumeDimension.z) {
            intersection.P = pos;
            intersection.N = calcNormal(d, axis);
            intersection.voxel = cell;
            intersection.t = 0;
            intersection.albedo = evaluateFinalColor(intersection.P, intersection.N, vec3(1, 0, 1));
            return true;
        }

        // DDA
        float tMin = min(nextT.x, min(nextT.y, nextT.z));
        axis = ivec3(nextT.x == tMin, nextT.y == tMin, nextT.z == tMin);
        nextT.xyz += deltaT.xyz * axis;
        cell += step * axis;

        pos = e + tMin * d;

        iteration++;
    } while (-1 <= cell.x && cell.x <= volumeDimension.x && -1 <= cell.y && cell.y <= volumeDimension.y && -1 <= cell.z && cell.z <= volumeDimension.z && iteration < MAX_ITERATIONS);

    return false;
}

// accelerated ray marching
bool sdfRayMarching(vec3 position, vec3 direction, out Intersection intersection) {
    vec3 e = position;
    {
        // intersect with bounding volume
        float tMin = 0.0;
        float tMax = FLT_MAX;
        if (!intersectAABB(vec3(-1, -1, -1), volumeDimension + vec3(1, 1, 1), position, direction, tMin, tMax)) {
            return false;
        }

        // move start position of ray to intersection with bounding volume
        e = position + tMin * direction;
    }
    vec3 pos = e;// current position
    vec3 d = direction;
    ivec3 step = ivec3(sign(d.x), sign(d.y), sign(d.z));

    // current cell index
    ivec3 cell = ivec3(floor(pos.x), floor(pos.y), floor(pos.z));
    // how much t increases when moving through one cell along each axis
    vec3 deltaT = vec3(d.x == 0 ? FLT_MAX : (sign(d.x) / d.x), d.y == 0 ? FLT_MAX : (sign(d.y) / d.y), d.z == 0 ? FLT_MAX : (sign(d.z) / d.z));
    // e+nextT.i*d is the minimal value, s.t. the ray intersects with the next cell on the i-axis
    vec3 nextT = vec3(d.x == 0 ? FLT_MAX : ((floor(e.x) + (sign(d.x) >= 0.0 ? 1 : 0) - e.x) / d.x),
    d.y == 0 ? FLT_MAX : ((floor(e.y) + (sign(d.y) >= 0.0 ? 1 : 0) - e.y) / d.y),
    d.z == 0 ? FLT_MAX : ((floor(e.z) + (sign(d.z) >= 0.0 ? 1 : 0) - e.z) / d.z));

    // initialize axis for lighting calculation of cells along the outside
    ivec3 axis = ivec3(0, 0, 0);
    if (nextT.x <= nextT.y && nextT.x <= nextT.z) {
        axis = ivec3(1, 0, 0);
    } else if (nextT.y <= nextT.x && nextT.y <= nextT.z) {
        axis = ivec3(0, 1, 0);
    } else {
        axis = ivec3(0, 0, 1);
    }

    int iteration = 0;
    do {
        uint tex = texelFetch(tex_volume, cell, 0).r;// get volume information of current cell
        if (tex > 0) {
            intersection.P = pos;
            intersection.N = calcNormal(d, axis);
            intersection.voxel = cell;
            intersection.t = 0;
            intersection.albedo = vec3(cellProperties[tex].red, cellProperties[tex].green, cellProperties[tex].blue);
            return true;
        }

        float sdf = texelFetch(tex_sdf, cell, 0).r - 3;// distance field value
        if (sdf <= 0) {
            // DDA
            float tMin = min(nextT.x, min(nextT.y, nextT.z));
            axis = ivec3(nextT.x == tMin, nextT.y == tMin, nextT.z == tMin);
            nextT.xyz += deltaT.xyz * axis;
            cell += step * axis;

            pos = e + tMin * d;
        } else {
            // fast traversal
            vec3 C = sdf / (abs(d.x) + abs(d.y) + abs(d.z)) * d;
            pos.xyz += C;// update current position
            cell = ivec3(floor(pos.x), floor(pos.y), floor(pos.z));
            nextT.xyz = vec3(d.x == 0 ? FLT_MAX : ((floor(pos.x) + (sign(d.x) >= 0.0 ? 1 : 0) - e.x) / d.x),
            d.y == 0 ? FLT_MAX : ((floor(pos.y) + (sign(d.y) >= 0.0 ? 1 : 0) - e.y) / d.y),
            d.z == 0 ? FLT_MAX : ((floor(pos.z) + (sign(d.z) >= 0.0 ? 1 : 0) - e.z) / d.z));
        }

        iteration++;
    } while (-1 <= cell.x && cell.x <= volumeDimension.x && -1 <= cell.y && cell.y <= volumeDimension.y && -1 <= cell.z && cell.z <= volumeDimension.z && iteration < MAX_ITERATIONS);

    return false;
}

/*
    === MAIN ===
*/
void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(albedoBuffer);
    if (pixel.x >= size.x || pixel.y >= size.y) {
        return;
    }

    vec2 position = (vec2(pixel) + vec2(0.5)) / vec2(size);
    vec3 direction = normalize(mix(mix(ray00, ray01, position.y), mix(ray10, ray11, position.y), position.x));
    Intersection isect;

    if (udbgSdf > 0) {
        if (!rayMarching(rayOrigin, direction, isect)) {
            writeBuffers(pixel, vec4(0), ivec3(1, 0, 0), BACKGROUND, 1.0, ivec3(0, 0, 0));
            return;
        }
        writeBuffers(pixel, vec4(isect.P, 1.0), isect.N, isect.albedo, calcDepth(isect.P), isect.voxel);
        return;
    }

    if (!sdfRayMarching(rayOrigin, direction, isect)) {
        writeBuffers(pixel, vec4(0), ivec3(1, 0, 0), BACKGROUND, 1.0, ivec3(0, 0, 0));
        return;
    }
    writeBuffers(pixel, vec4(isect.P, 1.0), isect.N, isect.albedo, calcDepth(isect.P), isect.voxel);
}