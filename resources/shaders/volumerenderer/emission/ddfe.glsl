/**
 * Directional RGB Distance Field Emission.
 * Sampling the already propagated emitted light during the render pass. See df/rgb/drgbdf_* for the precomputation step.
 * This shader is also used to sample RGB Distance Field Emission.
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, rgba32f) uniform image2D emissionBuffer;

struct CellProperties {
    float red;
    float green;
    float blue;
    float alpha;
    float emittance;
    float roughness;
};
layout(binding = 1, std430) buffer cellPropertiesBuffer {
    CellProperties cellProperties[];
};

uniform sampler2D tex_positionBuffer;// [x,y,z,valid], valid == 1 if encodes hit, 0 otherwise (background)
uniform isampler2D tex_normalBuffer;

uniform usampler3D tex_rgbdf;// r16i - [ 1bit | 5bit blue | 5bit green | 5bit red ]

layout(local_size_x = 16, local_size_y = 16) in;

#define RED_MASK uint(0x001F)
#define GREEN_MASK uint(0x03E0)
#define BLUE_MASK uint(0x7C00)

#define GREEN_SHIFT 5
#define BLUE_SHIFT 10

#define PI 3.14159265359
#define EPSILON 0.0001

ivec3 intToNormal(int n) {
    return ((1 & n) > 0 ? -1 : 1) * ivec3(sign(2 & n), sign(4 & n), sign(8 & n));
}

float rgbFalloff(float x) {
    return pow(x, 3);
}

    #define LIGHT_LEVELS 15.0// {0,1,..,15}
vec3 decompressRGB(ivec3 voxel) {
    uint value = texelFetch(tex_rgbdf, voxel, 0).r;
    vec3 rgb;
    rgb.r = (value & RED_MASK);
    rgb.g = (value & GREEN_MASK) >> GREEN_SHIFT;
    rgb.b = (value & BLUE_MASK) >> BLUE_SHIFT;
    rgb /= LIGHT_LEVELS;
    //    rgb.r = rgbFalloff(rgb.r); // apply (non-linear) falloff
    //    rgb.g = rgbFalloff(rgb.g);
    //    rgb.b = rgbFalloff(rgb.b);
    return rgb;
}

vec3 bilinearInterpolation(vec2 P, int z) {
    int rx = int(round(P.x));
    int x1 = rx >= P.x ? rx - 1 : rx;
    int x2 = rx >= P.x ? rx : rx + 1;
    int ry = int(round(P.y));
    int y1 = ry >= P.y ? ry - 1 : ry;
    int y2 = ry >= P.y ? ry : ry + 1;

    // x - bilinear interpolation
    float xFactor = P.x - x1;

    vec3 x1y1z = decompressRGB(ivec3(x1, y1, z));
    vec3 x2y1z = decompressRGB(ivec3(x2, y1, z));
    vec3 y1C = mix(x1y1z, x2y1z, xFactor);
    vec3 x1y2z = decompressRGB(ivec3(x1, y2, z));
    vec3 x2y2z = decompressRGB(ivec3(x2, y2, z));
    vec3 y2C = mix(x1y2z, x2y2z, xFactor);

    // y - bilinear interpolation
    float yFactor = P.y - y1;
    return mix(y1C, y2C, yFactor);
}

vec3 trilinearInterpolation(vec3 P) {
    int rz = int(round(P.z));
    int z1 = rz >= P.z ? rz - 1 : rz;
    int z2 = rz >= P.z ? rz : rz + 1;

    vec3 z1C = bilinearInterpolation(P.xy, z1);
    vec3 z2C = bilinearInterpolation(P.xy, z2);

    // z - bilinear interpolation
    float zFactor = P.z - z1;
    return mix(z1C, z2C, zFactor);
}

vec3 evaluateEmission(vec3 P, ivec3 N) {
    P = P + (0.5 + EPSILON) * N;
    P -= vec3(0.5); // correction for texture access
    return trilinearInterpolation(P);
}

void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(emissionBuffer);
    if (pixel.x >= size.x || pixel.y >= size.y) {
        return;
    }

    vec4 position = texelFetch(tex_positionBuffer, pixel, 0);
    if (position.a == 0) {
        imageStore(emissionBuffer, pixel, vec4(0, 0, 0, 0));
        return;
    }
    ivec3 normal = intToNormal(texelFetch(tex_normalBuffer, pixel, 0).r);

    vec3 emission = evaluateEmission(position.xyz, normal);
    imageStore(emissionBuffer, pixel, vec4(emission, 1));
}
