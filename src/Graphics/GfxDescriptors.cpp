#include "GfxDescriptors.hpp"

#include <cassert>
#include <stdexcept>

// *************** Descriptor Set Layout Builder *********************

GfxDescriptorSetLayout::Builder &GfxDescriptorSetLayout::Builder::addBinding(
    uint32_t binding, VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags, uint32_t count) {
  assert(bindings.count(binding) == 0 && "Binding already in use");
  VkDescriptorSetLayoutBinding layoutBinding{};
  layoutBinding.binding = binding;
  layoutBinding.descriptorType = descriptorType;
  layoutBinding.descriptorCount = count;
  layoutBinding.stageFlags = stageFlags;
  bindings[binding] = layoutBinding;
  return *this;
}

std::unique_ptr<GfxDescriptorSetLayout>
GfxDescriptorSetLayout::Builder::build() const {
  return std::make_unique<GfxDescriptorSetLayout>(gfxDevice, bindings);
}

// *************** Descriptor Set Layout *********************

GfxDescriptorSetLayout::GfxDescriptorSetLayout(
    GfxDevice &gfxDevice,
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : gfxDevice{gfxDevice}, bindings{bindings} {
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : bindings) {
    setLayoutBindings.push_back(kv.second);
  }

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
  descriptorSetLayoutInfo.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount =
      static_cast<uint32_t>(setLayoutBindings.size());
  descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

  if (vkCreateDescriptorSetLayout(gfxDevice.device(), &descriptorSetLayoutInfo,
                                  nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

GfxDescriptorSetLayout::~GfxDescriptorSetLayout() {
  vkDestroyDescriptorSetLayout(gfxDevice.device(), descriptorSetLayout,
                               nullptr);
}

// *************** Descriptor Pool Builder *********************

GfxDescriptorPool::Builder &
GfxDescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType,
                                        uint32_t count) {
  poolSizes.push_back({descriptorType, count});
  return *this;
}

GfxDescriptorPool::Builder &
GfxDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) {
  poolFlags = flags;
  return *this;
}
GfxDescriptorPool::Builder &
GfxDescriptorPool::Builder::setMaxSets(uint32_t count) {
  maxSets = count;
  return *this;
}

std::unique_ptr<GfxDescriptorPool> GfxDescriptorPool::Builder::build() const {
  return std::make_unique<GfxDescriptorPool>(gfxDevice, maxSets, poolFlags,
                                             poolSizes);
}

// *************** Descriptor Pool *********************

GfxDescriptorPool::GfxDescriptorPool(
    GfxDevice &gfxDevice, uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize> &poolSizes)
    : gfxDevice{gfxDevice} {
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;

  if (vkCreateDescriptorPool(gfxDevice.device(), &descriptorPoolInfo, nullptr,
                             &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

GfxDescriptorPool::~GfxDescriptorPool() {
  vkDestroyDescriptorPool(gfxDevice.device(), descriptorPool, nullptr);
}

bool GfxDescriptorPool::allocateDescriptor(
    const VkDescriptorSetLayout descriptorSetLayout,
    VkDescriptorSet &descriptor) const {
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;

  // Might want to create a "DescriptorPoolManager" class that handles this
  // case, and builds a new pool whenever an old pool fills up. But this is
  // beyond our current scope
  if (vkAllocateDescriptorSets(gfxDevice.device(), &allocInfo, &descriptor) !=
      VK_SUCCESS) {
    return false;
  }
  return true;
}

void GfxDescriptorPool::freeDescriptors(
    std::vector<VkDescriptorSet> &descriptors) const {
  vkFreeDescriptorSets(gfxDevice.device(), descriptorPool,
                       static_cast<uint32_t>(descriptors.size()),
                       descriptors.data());
}

void GfxDescriptorPool::resetPool() {
  vkResetDescriptorPool(gfxDevice.device(), descriptorPool, 0);
}

// *************** Descriptor Writer *********************

GfxDescriptorWriter::GfxDescriptorWriter(GfxDescriptorSetLayout &setLayout,
                                         GfxDescriptorPool &pool)
    : m_setLayout{setLayout}, m_pool{pool} {}

GfxDescriptorWriter &
GfxDescriptorWriter::writeBuffer(uint32_t binding,
                                 VkDescriptorBufferInfo *bufferInfo) {
  assert(m_setLayout.bindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  auto &bindingDescription = m_setLayout.bindings[binding];

  assert(bindingDescription.descriptorCount == 1 &&
         "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = bufferInfo;
  write.descriptorCount = 1;

  m_writes.push_back(write);
  return *this;
}

GfxDescriptorWriter &
GfxDescriptorWriter::writeImage(uint32_t binding,
                                VkDescriptorImageInfo *imageInfo) {
  assert(m_setLayout.bindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  auto &bindingDescription = m_setLayout.bindings[binding];

  assert(bindingDescription.descriptorCount == 1 &&
         "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pImageInfo = imageInfo;
  write.descriptorCount = 1;

  m_writes.push_back(write);
  return *this;
}

bool GfxDescriptorWriter::build(VkDescriptorSet &set) {
  bool success =
      m_pool.allocateDescriptor(m_setLayout.getDescriptorSetLayout(), set);
  if (!success) {
    return false;
  }
  overwrite(set);
  return true;
}

void GfxDescriptorWriter::overwrite(VkDescriptorSet &set) {
  for (auto &write : m_writes) {
    write.dstSet = set;
  }
  vkUpdateDescriptorSets(m_pool.gfxDevice.device(), m_writes.size(),
                         m_writes.data(), 0, nullptr);
}
