#version 460

layout(location = 0) in vec3 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler3D textureSampler;

void main() {
    outColor = texture(textureSampler, fragTexCoord);
}