#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out vec3 fragNormal;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionView;
    vec4 ambientLightColor;
    vec4 lightPos;
    vec4 lightColor;
} ubo;

layout(push_constant) uniform Push {
    mat4 model;
    mat4 normal;
} push;

void main() {
    vec4 worldPos = push.model * vec4(position, 1.0);
    gl_Position = ubo.projectionView * worldPos;

    fragNormal = normalize((push.normal * vec4(normal, 0.0)).xyz);
    fragWorldPos = worldPos.xyz;
    fragColor = color;
}