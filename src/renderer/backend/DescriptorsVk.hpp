#pragma once

#include "RenderDeviceVk.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

class DescriptorSetLayoutVk {
public:
  class Builder {
  public:
    Builder(RenderDeviceVk *device) : m_device{device} {}

    Builder &addBinding(uint32_t binding, vk::DescriptorType descriptorType,
                        vk::ShaderStageFlags stageFlags, uint32_t count = 1);
    std::unique_ptr<DescriptorSetLayoutVk> build() const;

  private:
    RenderDeviceVk *m_device;
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
  };

  DescriptorSetLayoutVk(
      RenderDeviceVk *device,
      std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings);
  ~DescriptorSetLayoutVk();
  DescriptorSetLayoutVk(const DescriptorSetLayoutVk &) = delete;
  DescriptorSetLayoutVk &operator=(const DescriptorSetLayoutVk &) = delete;

  vk::DescriptorSetLayout getDescriptorSetLayout() const {
    return descriptorSetLayout;
  }

private:
  RenderDeviceVk *m_device;
  vk::DescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings;

  friend class DescriptorWriterVk;
};

class DescriptorPoolVk {
public:
  class Builder {
  public:
    Builder(RenderDeviceVk *device) : m_device{device} {}

    Builder &addPoolSize(vk::DescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(vk::DescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<DescriptorPoolVk> build() const;

  private:
    RenderDeviceVk *m_device;
    std::vector<vk::DescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    vk::DescriptorPoolCreateFlags poolFlags{};
  };

  DescriptorPoolVk(RenderDeviceVk *device, uint32_t maxSets,
                   vk::DescriptorPoolCreateFlags poolFlags,
                   const std::vector<vk::DescriptorPoolSize> &poolSizes);
  ~DescriptorPoolVk();
  DescriptorPoolVk(const DescriptorPoolVk &) = delete;
  DescriptorPoolVk &operator=(const DescriptorPoolVk &) = delete;

  bool allocateDescriptor(const vk::DescriptorSetLayout descriptorSetLayout,
                          vk::DescriptorSet &descriptor) const;

  void freeDescriptors(std::vector<vk::DescriptorSet> &descriptors) const;

  void resetPool();

private:
  RenderDeviceVk *m_device;
  vk::DescriptorPool m_descriptorPool;

  friend class DescriptorWriterVk;
};

class DescriptorWriterVk {
public:
  DescriptorWriterVk(DescriptorSetLayoutVk &setLayout, DescriptorPoolVk &pool);

  DescriptorWriterVk &writeBuffer(uint32_t binding,
                                  vk::DescriptorBufferInfo *bufferInfo);
  DescriptorWriterVk &writeImage(uint32_t binding,
                                 vk::DescriptorImageInfo *imageInfo);

  bool build(vk::DescriptorSet &set);
  void overwrite(vk::DescriptorSet &set);

private:
  DescriptorSetLayoutVk &m_setLayout;
  DescriptorPoolVk &m_pool;
  std::vector<vk::WriteDescriptorSet> m_writes;
};
