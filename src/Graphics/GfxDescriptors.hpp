#pragma once

#include "GfxDevice.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

class GfxDescriptorSetLayout {
public:
  class Builder {
  public:
    Builder(GfxDevice &gfxDevice) : gfxDevice{gfxDevice} {}

    Builder &addBinding(uint32_t binding, VkDescriptorType descriptorType,
                        VkShaderStageFlags stageFlags, uint32_t count = 1);
    std::unique_ptr<GfxDescriptorSetLayout> build() const;

  private:
    GfxDevice &gfxDevice;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };

  GfxDescriptorSetLayout(
      GfxDevice &gfxDevice,
      std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~GfxDescriptorSetLayout();
  GfxDescriptorSetLayout(const GfxDescriptorSetLayout &) = delete;
  GfxDescriptorSetLayout &operator=(const GfxDescriptorSetLayout &) = delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const {
    return descriptorSetLayout;
  }

private:
  GfxDevice &gfxDevice;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

  friend class GfxDescriptorWriter;
};

class GfxDescriptorPool {
public:
  class Builder {
  public:
    Builder(GfxDevice &gfxDevice) : gfxDevice{gfxDevice} {}

    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<GfxDescriptorPool> build() const;

  private:
    GfxDevice &gfxDevice;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };

  GfxDescriptorPool(GfxDevice &gfxDevice, uint32_t maxSets,
                    VkDescriptorPoolCreateFlags poolFlags,
                    const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~GfxDescriptorPool();
  GfxDescriptorPool(const GfxDescriptorPool &) = delete;
  GfxDescriptorPool &operator=(const GfxDescriptorPool &) = delete;

  bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout,
                          VkDescriptorSet &descriptor) const;

  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

  void resetPool();

private:
  GfxDevice &gfxDevice;
  VkDescriptorPool descriptorPool;

  friend class GfxDescriptorWriter;
};

class GfxDescriptorWriter {
public:
  GfxDescriptorWriter(GfxDescriptorSetLayout &setLayout,
                      GfxDescriptorPool &pool);

  GfxDescriptorWriter &writeBuffer(uint32_t binding,
                                   VkDescriptorBufferInfo *bufferInfo);
  GfxDescriptorWriter &writeImage(uint32_t binding,
                                  VkDescriptorImageInfo *imageInfo);

  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);

private:
  GfxDescriptorSetLayout &setLayout;
  GfxDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};
