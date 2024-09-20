#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

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

    vec3 normalWorldSpace = normalize((push.normal * vec4(normal, 0.0)).xyz);

    vec3 directionTolight = ubo.lightPos.xyz - worldPos.xyz;
    float attenuation = 1.0 / dot(directionTolight, directionTolight);

    vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
    vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 diffuseLight = lightColor * max(dot(normalWorldSpace, normalize(directionTolight)), 0);

    fragColor = (ambientLight + diffuseLight) * color;
}