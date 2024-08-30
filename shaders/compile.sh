#!/bin/bash

# Получаем путь к текущему скрипту
SCRIPT_PATH=$(readlink -f "$0")

# Получаем путь к директории с файлами шейдеров
SHADER_DIR=$(dirname "$SCRIPT_PATH")

for file in "$SHADER_DIR"/*.vert "$SHADER_DIR"/*.frag; do
  glslc -o ${file}.spv $file
done