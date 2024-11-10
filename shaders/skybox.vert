#version 460

layout(location = 0) in vec3 pos;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 view;
    mat4 projection;
    mat4 projectionView;
} ubo;

layout(location = 0) out vec4 outColor;

void main() {
	gl_Position = ubo.projection * vec4(pos, 1.0);
	outColor = vec4(1.0);
}