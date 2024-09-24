// #include "TextureHandlerVk.hpp"
// #include <stb_image.h>

// void TextureHandlerVk::loadFile(const std::string &filename, Texture &texture,
//                                 TextureID textureID) {

//   int channels;

//   void *pixels = stbi_load(filename.c_str(), &texture.width, &texture.height,
//                            &channels, STBI_rgb_alpha);

//   gli::texture gliTexture;

//   // If stbi could open this file
//   if (pixels != nullptr) {
//     // This is hardcoded to 4 instead of channels since STBI is loading it as
//     // STBI_rgb_alpha, making it 4 channels
//     texture.uploadSize = texture.width * texture.height * 4;
//     texture.format = VK_FORMAT_R8G8B8A8_UNORM;
//     texture.layers = 1; // If we are not loading using gli we don't support
//                         // layers, so don't bother with it

//     texture.mipLevels =
//         static_cast<u32>(
//             std::floor(std::log2(std::max(texture.width, texture.height)))) +
//         1;
//     u32 totalSize = 0;

//     u32 width = texture.width;
//     u32 height = texture.height;

//     for (size_t i = 0; i < texture.mipLevels; i++) {
//       u32 size = width * height * 4;
//       totalSize += size;

//       if (width > 1)
//         width /= 2;
//       if (height > 1)
//         height /= 2;
//     }
//     texture.totalSize = totalSize;
//     texture.numGeneratedMipLevels = texture.mipLevels - 1;
//   }

//   {
//     ZoneScopedN("CreateTexture");
//     // Create texture
//     CreateTexture(texture);

//     // Create upload buffer
//     auto uploadBuffer = _uploadBufferHandler->CreateUploadBuffer(
//         textureID, 0, texture.uploadSize, texture.numGeneratedMipLevels);

//     // Copy data to upload buffer
//     memcpy(uploadBuffer->mappedMemory, pixels, texture.uploadSize);
//   }
// }