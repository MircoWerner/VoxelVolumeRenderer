/**
 * Voxel Distance Field Cone Traced Ambient Occlusion.
 * Our combined AO method: VCTAO + DFAO + LVAO.
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, r32f) uniform image2D ambientOcclusionBuffer;

uniform sampler2D tex_positionBuffer;// [x,y,z,valid], valid == 1 if encodes hit, 0 otherwise (background)
uniform isampler2D tex_normalBuffer;
uniform isampler2D tex_voxelBuffer;

uniform usampler3D tex_volume;
uniform sampler3D tex_sdf;// city block distance
uniform ivec3 volumeDimension;

uniform int coneTraceSteps;
uniform float coneTraceStepSize;
uniform float coneApertureAngle;// radians
uniform float attenuation;

layout(local_size_x = 16, local_size_y = 16) in;

#define PI 3.14159265359

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

float max3f(float a, float b, float c) {
    return max(a, max(b, c));
}

float distancePositionAABB(vec3 P, ivec3 aabbMin, ivec3 aabbMax) {
    return max3f(P.x - aabbMax.x, 0, aabbMin.x - P.x)
    + max3f(P.y - aabbMax.y, 0, aabbMin.y - P.y)
    + max3f(P.z - aabbMax.z, 0, aabbMin.z - P.z);
}

float sdf(vec3 P, vec3 coordinate, float lod) {
    float sdfValue = textureLod(tex_sdf, coordinate, lod).r;
    sdfValue += distancePositionAABB(P, ivec3(0), volumeDimension);// add distance to the AABB if sample is outside
    return sdfValue;
}

float attenuate(float distance) {
    return 1.0 / (1.0 + distance / attenuation);
}

float coneTraceDF(vec3 P, vec3 N, vec3 dir) {
    vec3 INV_TEXTURE = 1.0 / volumeDimension;

    float cosTheta = max(0.0, dot(N, dir));
    float alpha = 0.0;
    float tanHalfAperture = tan(coneApertureAngle / 2.0);

    float distance = coneTraceStepSize;
    for (int i = 0; i < coneTraceSteps && alpha <= 0.99; i++) {
        vec3 position = P + distance * dir;
        if (position.x < 0 || position.x > volumeDimension.x || position.y < 0 || position.y > volumeDimension.y || position.z < 0 && position.z > volumeDimension.z) {
            break;
        }
        float coneDiameter = max(1.0, 2.0 * tanHalfAperture * distance);
        float sdfValue = sdf(P + distance * dir - 0.5 * N, (P + distance * dir - 0.5 * N) * INV_TEXTURE, log2(coneDiameter));
        float convAlpha = clamp(distance * cosTheta - sdfValue, 0, distance) / distance;// convert sdf value to alpha value
        float alphaStep = coneTraceStepSize * convAlpha;// correct alpha value
        alpha += attenuate(distance) * (1.0 - alpha) * alphaStep;// front to back accumulation, add attenuation
        distance += (1.0 + coneDiameter) * coneTraceStepSize;// increase distance
    }
    return alpha;
}

float dcao(vec3 P, vec3 N) {
    float alpha = 0.0;
    alpha += coneTraceDF(P, N, N);
    alpha += coneTraceDF(P, N, createDirection(0.785398, 0.785398, N));
    alpha += coneTraceDF(P, N, createDirection(2.35619, 0.785398, N));
    alpha += coneTraceDF(P, N, createDirection(3.92699, 0.785398, N));
    alpha += coneTraceDF(P, N, createDirection(5.49779, 0.785398, N));
    return 1.0 - alpha / 5.0;
}

int occ(ivec3 coord) {
    return texelFetch(tex_volume, coord, 0).r == 0 ? 0 : 1;
}

// https://iquilezles.org/articles/voxellines/
float lvao(vec3 position, ivec3 normal, ivec3 voxel) {
    ivec3 tangent = normal.x != 0 ? ivec3(0, 1, 0) : ivec3(1, 0, 0);
    ivec3 bitangent = normal.z != 0 ? ivec3(0, 1, 0) : ivec3(0, 0, 1);

    ivec3 cx = voxel + normal + tangent;
    ivec3 cy = voxel + normal - tangent;
    ivec3 cz = voxel + normal + bitangent;
    ivec3 cw = voxel + normal - bitangent;
    ivec3 dx = voxel + normal + tangent + bitangent;
    ivec3 dy = voxel + normal - tangent + bitangent;
    ivec3 dz = voxel + normal - tangent - bitangent;
    ivec3 dw = voxel + normal + tangent - bitangent;
    vec4 vc = ivec4(occ(cx), occ(cy), occ(cz), occ(cw));
    vec4 vd = ivec4(occ(dx), occ(dy), occ(dz), occ(dw));

    vec3 frac = fract(position);
    vec2 uv = normal.x != 0 ? frac.yz : (normal.y != 0 ? frac.xz : frac.xy);
    vec2 st = 1.0 - uv;
    vec4 wa = vec4(uv.x, st.x, uv.y, st.y) * vc;// edges
    vec4 wb = vec4(uv.x * uv.y, st.x * uv.y, st.x * st.y, uv.x * st.y) * vd * (1.0 - vc.xzyw) * (1.0 - vc.zywx);// corners

    float ao = 1.0 - (wa.x + wa.y + wa.z + wa.w + wb.x + wb.y + wb.z + wb.w);
    return (ao + 1.0) / 2.0;// shift ao values to make the scene brighter, works well in practice
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
    ivec3 voxel = texelFetch(tex_voxelBuffer, pixel, 0).xyz;

    float ao1 = lvao(position.xyz, normal, voxel);
    float ao2 = dcao(position.xyz, normal);
    float ao = min(ao1, ao2);// smooth transition

    imageStore(ambientOcclusionBuffer, pixel, vec4(ao, 0, 0, 0));
}
