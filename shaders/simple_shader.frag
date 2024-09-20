#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

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
    vec3 directionTolight = ubo.lightPos.xyz - fragWorldPos;
    float attenuation = 1.0 / dot(directionTolight, directionTolight);

    vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
    vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 diffuseLight = lightColor * max(dot(normalize(fragNormal), normalize(directionTolight)), 0);

    outColor = vec4((ambientLight + diffuseLight) * fragColor, 1.0);
}