#version 460

layout(location = 0) in uint positionAndTexCoords;
layout(location = 1) in float texIdx;

layout(location = 0) out vec3 fragTexCoord;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 view;
    mat4 projection;
    mat4 projectionView;
} ubo;

layout(push_constant) uniform Push {
    vec2 chunkPos;
} push;

vec3 decompressPos(uint pos) {
    return vec3(pos & 0xFF, pos >> 8 & 0xFF, pos >> 16 & 0xFF);
}

vec3 decompressTexCoords(uint pos, float texIdx) {
    return vec3(pos >> 24 & 0x1, pos >> 25 & 0x1, texIdx);
}

void main() {
    vec4 worldPos = vec4(push.chunkPos.x, 0.0, push.chunkPos.y, 0.0) + vec4(decompressPos(positionAndTexCoords), 1.0);
    gl_Position = ubo.projectionView * worldPos;
    fragTexCoord = decompressTexCoords(positionAndTexCoords, texIdx);
}