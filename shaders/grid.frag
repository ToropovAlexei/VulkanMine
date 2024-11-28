#version 460

layout(location = 0) in vec3 worldPos;
layout(location = 0) out vec4 outColor;

void main() {
    // Создаем базовую сетку
    vec2 coord = worldPos.xz;
    vec2 grid = abs(fract(coord) - 0.5);
    float line = min(grid.x, grid.y);
    
    // Базовый цвет сетки
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0);
    
    // Если близко к линии сетки, делаем ярче
    if(line < 0.1) {
        color.rgb = vec3(0.8);
    }
    
    // Оси координат
    if(abs(worldPos.x) < 0.1) {
        color = vec4(1.0, 0.0, 0.0, 1.0); // X - красная
    }
    if(abs(worldPos.z) < 0.1) {
        color = vec4(0.0, 0.0, 1.0, 1.0); // Z - синяя
    }
    
    outColor = color;
}