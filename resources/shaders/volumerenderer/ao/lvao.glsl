/**
 * Local Voxel Ambient Occlusion.
 * Reference: https://iquilezles.org/articles/voxellines/
 *
 * @author Mirco Werner
 */
#version 430 core

layout(binding = 0, r32f) uniform image2D ambientOcclusionBuffer;

uniform sampler2D tex_positionBuffer;// [x,y,z,valid], valid == 1 if encodes hit, 0 otherwise (background)
uniform isampler2D tex_normalBuffer;
uniform isampler2D tex_voxelBuffer;

uniform usampler3D tex_volume;

layout(local_size_x = 16, local_size_y = 16) in;

ivec3 intToNormal(int n) {
    return ((1 & n) > 0 ? -1 : 1) * ivec3(sign(2 & n), sign(4 & n), sign(8 & n));
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
    vec4 c = ivec4(occ(cx), occ(cy), occ(cz), occ(cw));
    vec4 d = ivec4(occ(dx), occ(dy), occ(dz), occ(dw));

    vec3 frac = fract(position);
    vec2 uv = normal.x != 0 ? frac.yz : (normal.y != 0 ? frac.xz : frac.xy);
    vec2 st = 1.0 - uv;
    vec4 wc = vec4(uv.x, st.x, uv.y, st.y) * c;// edges
    vec4 wd = vec4(uv.x * uv.y, st.x * uv.y, st.x * st.y, uv.x * st.y) * d * (1.0 - c.xzyw) * (1.0 - c.zywx);// corners

    float ao = 1.0 - (wc.x + wc.y + wc.z + wc.w + wd.x + wd.y + wd.z + wd.w);
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

    float ao = lvao(position.xyz, normal, voxel);

    imageStore(ambientOcclusionBuffer, pixel, vec4(ao, 0, 0, 0));
}
