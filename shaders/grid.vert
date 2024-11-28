#version 460

layout(location = 0) in vec3 pos;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 view;
    mat4 projection;
    mat4 projectionView;
    vec3 cameraPosition;
    float dayTime;
} ubo;

layout(location = 0) out vec3 worldPos;

void main() {
    // Просто передаем мировые координаты во фрагментный шейдер
    worldPos = pos;
    
    // Преобразуем позицию через матрицу проекции-вида
    gl_Position = ubo.projectionView * vec4(pos, 1.0);
}