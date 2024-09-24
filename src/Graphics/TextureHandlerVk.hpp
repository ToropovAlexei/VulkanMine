// #pragma once

// #include "vk_mem_alloc.h"
// #include <string>
// #include <vulkan/vulkan_core.h>

// class TextureHandlerVk {
// public:
//   struct Texture {
//     bool loaded = true;
//     u64 hash;

//     TextureID::type textureIndex;

//     i32 width;
//     i32 height;
//     i32 layers;
//     i32 mipLevels;
//     i32 numGeneratedMipLevels = 0;

//     VkFormat format;
//     size_t uploadSize;
//     size_t totalSize; // Including any possibly generated mips

//     VmaAllocation allocation;
//     VkImage image;
//     VkImageView imageView;
//     VkDescriptorSet imguiTextureHandle = 0;

//     std::string debugName = "";

//     bool layoutUndefined = true;
//   };

// public:
//   void loadFile(const std::string &filename, Texture &texture,
//                 TextureID textureID);
// };