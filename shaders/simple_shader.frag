#version 460

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    mat4 transform;
    mat4 model;
} push;

void main() {
    outColor = vec4(fragColor * vec3(1.0, 1.5, 1.2), 1.0);
}