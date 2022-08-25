/**
 * Fragment shader renders the data from the screen sized buffers.
 *
 * @author Mirco Werner
 */
#version 430 core

in vec2 tex;

uniform sampler2D tex_albedoBuffer;
uniform sampler2D tex_ambientOcclusionBuffer;
uniform sampler2D tex_emissionBuffer;

uniform isampler2D tex_normalBuffer;
uniform sampler2D tex_depthBuffer;

uniform bool ambientOcclusionEnabled;
uniform bool emissionEnabled;
uniform float emissionFactor;

uniform int dbgMode;// 0 - disabled, 1 - normal, 2 - depth, 3 - albedo, 4 - ambientocclusion, 5 - emission

uniform float nearPlane;
uniform float farPlane;

out vec4 fragColor;

float linearizeDepth(float depth) {
    return 2.0 * nearPlane * farPlane / (farPlane + nearPlane - (2.0 * depth - 1.0) * (farPlane - nearPlane));
}

ivec3 intToNormal(int n) {
    return ((1 & n) > 0 ? -1 : 1) * ivec3(sign(2 & n), sign(4 & n), sign(8 & n));
}

void main() {
    if (dbgMode > 0) {
        if (dbgMode == 1) {
            fragColor = vec4(abs(intToNormal(texture(tex_normalBuffer, tex).r)), 1.0);
        } else if (dbgMode == 2) {
            float depth = min(1.0, linearizeDepth(texture(tex_depthBuffer, tex).r) / 400.0);
            fragColor = vec4(depth, depth, depth, 1.0);
        } else if (dbgMode == 3) {
            fragColor = vec4(texture(tex_albedoBuffer, tex).rgb, 1.0);
        } else if (dbgMode == 4) {
            float ambientOcclusion = ambientOcclusionEnabled ? texture(tex_ambientOcclusionBuffer, tex).r : 1.0;
            fragColor = vec4(ambientOcclusion, ambientOcclusion, ambientOcclusion, 1.0);
        } else if (dbgMode == 5) {
            vec4 emission = emissionEnabled ? emissionFactor * texture(tex_emissionBuffer, tex) : vec4(0);
            fragColor = vec4(emission);
        }
        return;
    }

    vec3 albedo = texture(tex_albedoBuffer, tex).rgb;
    float ambientOcclusion = ambientOcclusionEnabled ? texture(tex_ambientOcclusionBuffer, tex).r : 1.0;
    vec4 emission = emissionEnabled ? emissionFactor * texture(tex_emissionBuffer, tex) : vec4(0);
    fragColor = (vec4(ambientOcclusion * albedo, 1.0) + emission);
}