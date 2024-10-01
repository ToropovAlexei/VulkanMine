#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionView;
} ubo;

layout(push_constant) uniform Push {
    mat4 model;
} push;

void main() {
    vec4 worldPos = push.model * vec4(position, 1.0);
    gl_Position = ubo.projectionView * worldPos;

    fragColor = color;
}