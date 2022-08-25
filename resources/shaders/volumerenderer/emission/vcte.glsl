/**
 * Voxel Cone Traced Emission.
 * Reference: https://doi.org/10.1111/j.1467-8659.2011.02063.x
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

uniform usampler3D tex_volume;
uniform sampler2D tex_positionBuffer;// [x,y,z,valid], valid == 1 if encodes hit, 0 otherwise (background)
uniform isampler2D tex_normalBuffer;
uniform isampler2D tex_voxelBuffer;

uniform ivec3 volumeDimension;
uniform vec3 rayOrigin;

uniform sampler3D tex_albedo;//rgb light at emissive sources
uniform sampler3D tex_alpha;// local occlusion values

uniform bool calcDiffuse;
uniform int diffuseConeTraceSteps;
uniform float diffuseConeTraceStepSize;
uniform float diffuseConeApertureAngle;// radians

uniform bool calcSpecular;
uniform int specularConeTraceSteps;
uniform float specularConeTraceStepSize;
uniform float specularConeApertureAngle;// radians

layout(local_size_x = 16, local_size_y = 16) in;

#define PI 3.14159265359
#define EPSILON 0.001

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

vec3 coneTrace(vec3 P, vec3 N, vec3 direction, int coneTraceSteps, float aperture, float stepSize) {
    vec3 INV_TEXTURE = 1.0 / volumeDimension;

    vec4 light = vec4(0.0);
    float tanHalfAperture = tan(aperture / 2.0);

    float distance = stepSize;
    float distanceIncrease = distance;
    for (int i = 0; i < coneTraceSteps && light.a <= 0.99; i++) {
        vec3 position = P + distance * direction;
        if (position.x < 0 || position.x > volumeDimension.x || position.y < 0 || position.y > volumeDimension.y || position.z < 0 && position.z > volumeDimension.z) {
            break;
        }
        float coneDiameter = max(1.0, 2.0 * tanHalfAperture * distance);
        vec3 rgb = textureLod(tex_albedo, position * INV_TEXTURE, max(0.0, log2(coneDiameter) - 1)).rgb;// albedo lookup at mip map level
        float a = textureLod(tex_alpha, position * INV_TEXTURE, log2(coneDiameter)).r;// alpha lookup at mip map level
        vec4 lightStep = vec4(rgb, a);
        vec4 base = 1 - lightStep.rgba;
        float exponent = distanceIncrease / coneDiameter;
        light.rgba += (1.0 - light.a) * (1 - vec4(pow(base.x, exponent), pow(base.y, exponent), pow(base.z, exponent), pow(base.w, exponent)));// front to back accumulation, add attenuation, correct light values
        distanceIncrease = (1.0 + coneDiameter) * stepSize;
        distance += distanceIncrease;// increase distance
    }
    return light.rgb;
}

vec3 diffuseLight(vec3 P, vec3 N) {
    vec3 light = vec3(0.0);

    light += coneTrace(P, N, N, diffuseConeTraceSteps, diffuseConeApertureAngle, diffuseConeTraceStepSize);
    light += coneTrace(P, N, createDirection(0.785398, 0.785398, N), diffuseConeTraceSteps, diffuseConeApertureAngle, diffuseConeTraceStepSize);
    light += coneTrace(P, N, createDirection(2.35619, 0.785398, N), diffuseConeTraceSteps, diffuseConeApertureAngle, diffuseConeTraceStepSize);
    light += coneTrace(P, N, createDirection(3.92699, 0.785398, N), diffuseConeTraceSteps, diffuseConeApertureAngle, diffuseConeTraceStepSize);
    light += coneTrace(P, N, createDirection(5.49779, 0.785398, N), diffuseConeTraceSteps, diffuseConeApertureAngle, diffuseConeTraceStepSize);

    return light / 5.0;
}

vec3 specularLight(vec3 P, vec3 N, vec3 R) {
    return coneTrace(P, N, R, specularConeTraceSteps, specularConeApertureAngle, specularConeTraceStepSize);
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

    vec3 emission = vec3(0);
    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);

    if (calcDiffuse) {
        diffuse += diffuseLight(position.xyz, normal);
    }
    if (calcSpecular) {
        specular += specularLight(position.xyz, normal, normalize(reflect(position.xyz - rayOrigin, normal)));
    }

    if (calcDiffuse && calcSpecular) {
        ivec3 voxel = texelFetch(tex_voxelBuffer, pixel, 0).xyz;
        uint cellId = texelFetch(tex_volume, voxel, 0).r;
        float roughness = cellProperties[cellId].roughness;
        emission = roughness * diffuse + (1.0 - roughness) * specular;
    } else {
        emission = diffuse + specular;
    }

    imageStore(emissionBuffer, pixel, vec4(emission, 1));
}
