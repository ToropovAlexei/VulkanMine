#include "DescriptorsVk.hpp"
#include <cassert>

// *************** Descriptor Set Layout Builder *********************

DescriptorSetLayoutVk::Builder &DescriptorSetLayoutVk::Builder::addBinding(
    uint32_t binding, vk::DescriptorType descriptorType,
    vk::ShaderStageFlags stageFlags, uint32_t count) {
  assert(bindings.count(binding) == 0 && "Binding already in use");
  vk::DescriptorSetLayoutBinding layoutBinding = {
      .binding = binding,
      .descriptorType = descriptorType,
      .descriptorCount = count,
      .stageFlags = stageFlags,
  };

  bindings[binding] = layoutBinding;
  return *this;
}

std::unique_ptr<DescriptorSetLayoutVk>
DescriptorSetLayoutVk::Builder::build() const {
  return std::make_unique<DescriptorSetLayoutVk>(m_device, bindings);
}

// *************** Descriptor Set Layout *********************

DescriptorSetLayoutVk::DescriptorSetLayoutVk(
    RenderDeviceVk *device,
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings)
    : m_device{device}, bindings{bindings} {
  std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : bindings) {
    setLayoutBindings.push_back(kv.second);
  }

  vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {
      .bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
      .pBindings = setLayoutBindings.data(),
  };

  descriptorSetLayout =
      m_device->getDevice().createDescriptorSetLayout(descriptorSetLayoutInfo);
}

DescriptorSetLayoutVk::~DescriptorSetLayoutVk() {
  m_device->getDevice().destroyDescriptorSetLayout(descriptorSetLayout);
}

// *************** Descriptor Pool Builder *********************

DescriptorPoolVk::Builder &
DescriptorPoolVk::Builder::addPoolSize(vk::DescriptorType descriptorType,
                                       uint32_t count) {
  poolSizes.push_back({descriptorType, count});
  return *this;
}

DescriptorPoolVk::Builder &
DescriptorPoolVk::Builder::setPoolFlags(vk::DescriptorPoolCreateFlags flags) {
  poolFlags = flags;
  return *this;
}
DescriptorPoolVk::Builder &
DescriptorPoolVk::Builder::setMaxSets(uint32_t count) {
  maxSets = count;
  return *this;
}

std::unique_ptr<DescriptorPoolVk> DescriptorPoolVk::Builder::build() const {
  return std::make_unique<DescriptorPoolVk>(m_device, maxSets, poolFlags,
                                            poolSizes);
}

// *************** Descriptor Pool *********************

DescriptorPoolVk::DescriptorPoolVk(
    RenderDeviceVk *device, uint32_t maxSets,
    vk::DescriptorPoolCreateFlags poolFlags,
    const std::vector<vk::DescriptorPoolSize> &poolSizes)
    : m_device{device} {
  vk::DescriptorPoolCreateInfo descriptorPoolInfo = {
      .flags = poolFlags,
      .maxSets = maxSets,
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.data(),
  };

  m_descriptorPool =
      m_device->getDevice().createDescriptorPool(descriptorPoolInfo);
}

DescriptorPoolVk::~DescriptorPoolVk() {
  m_device->getDevice().destroyDescriptorPool(m_descriptorPool);
}

bool DescriptorPoolVk::allocateDescriptor(
    const vk::DescriptorSetLayout descriptorSetLayout,
    vk::DescriptorSet &descriptor) const {
  vk::DescriptorSetAllocateInfo allocInfo = {
      .descriptorPool = m_descriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptorSetLayout,
  };

  // Might want to create a "DescriptorPoolManager" class that handles this
  // case, and builds a new pool whenever an old pool fills up. But this is
  // beyond our current scope
  auto res =
      m_device->getDevice().allocateDescriptorSets(&allocInfo, &descriptor);
  if (res != vk::Result::eSuccess) {
    return false;
  }
  return true;
}

void DescriptorPoolVk::freeDescriptors(
    std::vector<vk::DescriptorSet> &descriptors) const {
  m_device->getDevice().freeDescriptorSets(m_descriptorPool, descriptors);
}

void DescriptorPoolVk::resetPool() {
  m_device->getDevice().resetDescriptorPool(m_descriptorPool);
}

// *************** Descriptor Writer *********************

DescriptorWriterVk::DescriptorWriterVk(DescriptorSetLayoutVk &setLayout,
                                       DescriptorPoolVk &pool)
    : m_setLayout{setLayout}, m_pool{pool} {}

DescriptorWriterVk &
DescriptorWriterVk::writeBuffer(uint32_t binding,
                                vk::DescriptorBufferInfo *bufferInfo) {
  assert(m_setLayout.bindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  auto &bindingDescription = m_setLayout.bindings[binding];

  assert(bindingDescription.descriptorCount == 1 &&
         "Binding single descriptor info, but binding expects multiple");

  vk::WriteDescriptorSet write = {
      .dstBinding = binding,
      .descriptorCount = 1,
      .descriptorType = bindingDescription.descriptorType,
      .pBufferInfo = bufferInfo,
  };

  m_writes.push_back(write);
  return *this;
}

DescriptorWriterVk &
DescriptorWriterVk::writeImage(uint32_t binding,
                               vk::DescriptorImageInfo *imageInfo) {
  assert(m_setLayout.bindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  auto &bindingDescription = m_setLayout.bindings[binding];

  assert(bindingDescription.descriptorCount == 1 &&
         "Binding single descriptor info, but binding expects multiple");

  vk::WriteDescriptorSet write = {
      .dstBinding = binding,
      .descriptorCount = 1,
      .descriptorType = bindingDescription.descriptorType,
      .pImageInfo = imageInfo,
  };

  m_writes.push_back(write);
  return *this;
}

bool DescriptorWriterVk::build(vk::DescriptorSet &set) {
  bool success =
      m_pool.allocateDescriptor(m_setLayout.getDescriptorSetLayout(), set);
  if (!success) {
    return false;
  }
  overwrite(set);
  return true;
}

void DescriptorWriterVk::overwrite(vk::DescriptorSet &set) {
  for (auto &write : m_writes) {
    write.dstSet = set;
  }
  m_pool.m_device->getDevice().updateDescriptorSets(m_writes, 0);
}
