#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 inTexCoord;

layout(location = 0) out vec3 fragTexCoord;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionView;
} ubo;

layout(push_constant) uniform Push {
    vec2 chunkPos;
} push;

void main() {
    vec4 worldPos = vec4(push.chunkPos.x, 0.0, push.chunkPos.y, 0.0) + vec4(position, 1.0);
    gl_Position = ubo.projectionView * worldPos;
    fragTexCoord = inTexCoord;
}