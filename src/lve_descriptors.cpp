#include "lve_descriptors.h"

// Standard includes
#include <cassert>
#include <stdexcept>

namespace lve
{
    // *************** Descriptor Set Layout Builder *********************

    lve_descriptor_set_layout::builder &lve_descriptor_set_layout::builder::add_binding(
        uint32_t binding,
        VkDescriptorType descriptor_type,
        VkShaderStageFlags stage_flags,
        uint32_t count)
    {
        assert(bindings_.count(binding) == 0 and "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding         = binding;
        layoutBinding.descriptorType  = descriptor_type;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags      = stage_flags;
        bindings_[binding]            = layoutBinding;
        return *this;
    }

    auto lve_descriptor_set_layout::builder::build() const -> std::unique_ptr<lve_descriptor_set_layout>
    {
        return std::make_unique<lve_descriptor_set_layout>(device_, bindings_);
    }

    // *************** Descriptor Set Layout *********************

    lve_descriptor_set_layout::lve_descriptor_set_layout(
        lve_device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : device_{device}, bindings{bindings}
    {
        std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};
        for (auto kv : bindings)
        {
            set_layout_bindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
        descriptor_set_layout_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
        descriptor_set_layout_info.pBindings    = set_layout_bindings.data();

        if (vkCreateDescriptorSetLayout(
            device.device(),
            &descriptor_set_layout_info,
            nullptr,
            &descriptor_set_layout_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    lve_descriptor_set_layout::~lve_descriptor_set_layout()
    {
        vkDestroyDescriptorSetLayout(device_.device(), descriptor_set_layout_, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    auto lve_descriptor_pool::builder::add_pool_size(VkDescriptorType const descriptor_type, uint32_t count) -> lve_descriptor_pool::builder &
    {
        pool_sizes_.push_back({descriptor_type, count});
        return *this;
    }

    auto lve_descriptor_pool::builder::set_pool_flags(VkDescriptorPoolCreateFlags flags) -> lve_descriptor_pool::builder &
    {
        pool_flags_ = flags;
        return *this;
    }

    auto lve_descriptor_pool::builder::set_max_sets(uint32_t count) -> lve_descriptor_pool::builder &
    {
        max_sets_ = count;
        return *this;
    }

    auto lve_descriptor_pool::builder::build() const -> std::unique_ptr<lve_descriptor_pool>
    {
        return std::make_unique<lve_descriptor_pool>(device_, max_sets_, pool_flags_, pool_sizes_);
    }

    // *************** Descriptor Pool *********************

    lve_descriptor_pool::lve_descriptor_pool(
        lve_device &device,
        uint32_t max_sets,
        VkDescriptorPoolCreateFlags pool_flags,
        const std::vector<VkDescriptorPoolSize> &pool_sizes)
        : device_{device}
    {
        VkDescriptorPoolCreateInfo descriptor_pool_info{};
        descriptor_pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        descriptor_pool_info.pPoolSizes    = pool_sizes.data();
        descriptor_pool_info.maxSets       = max_sets;
        descriptor_pool_info.flags         = pool_flags;

        if (vkCreateDescriptorPool(device.device(), &descriptor_pool_info, nullptr, &descriptor_pool_) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    lve_descriptor_pool::~lve_descriptor_pool()
    {
        vkDestroyDescriptorPool(device_.device(), descriptor_pool_, nullptr);
    }

    auto lve_descriptor_pool::allocate_descriptor(const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet &descriptor) const -> bool
    {
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool     = descriptor_pool_;
        alloc_info.pSetLayouts        = &descriptor_set_layout;
        alloc_info.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(device_.device(), &alloc_info, &descriptor) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }

    void lve_descriptor_pool::free_descriptors(std::vector<VkDescriptorSet> &descriptors) const
    {
        vkFreeDescriptorSets(
            device_.device(),
            descriptor_pool_,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void lve_descriptor_pool::reset_pool()
    {
        vkResetDescriptorPool(device_.device(), descriptor_pool_, 0);
    }

    // *************** Descriptor Writer *********************

    lve_descriptor_writer::lve_descriptor_writer(lve_descriptor_set_layout &set_layout, lve_descriptor_pool &pool)
        : set_layout_{set_layout}, pool_{pool}
    {
    }

    auto lve_descriptor_writer::write_buffer(uint32_t binding, VkDescriptorBufferInfo *buffer_info) -> lve_descriptor_writer &
    {
        assert(set_layout_.bindings.count(binding) == 1 and "Layout does not contain specified binding");

        auto &bindingDescription = set_layout_.bindings[binding];

        assert(bindingDescription.descriptorCount == 1 and "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType  = bindingDescription.descriptorType;
        write.dstBinding      = binding;
        write.pBufferInfo     = buffer_info;
        write.descriptorCount = 1;

        writes_.push_back(write);
        return *this;
    }

    auto lve_descriptor_writer::write_image(uint32_t binding, VkDescriptorImageInfo *image_info) -> lve_descriptor_writer &
    {
        assert(set_layout_.bindings.count(binding) == 1 and "Layout does not contain specified binding");

        auto &binding_description = set_layout_.bindings[binding];

        assert(binding_description.descriptorCount == 1 and "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType  = binding_description.descriptorType;
        write.dstBinding      = binding;
        write.pImageInfo      = image_info;
        write.descriptorCount = 1;

        writes_.push_back(write);
        return *this;
    }

    auto lve_descriptor_writer::build(VkDescriptorSet &set) -> bool
    {
        bool success = pool_.allocate_descriptor(set_layout_.get_descriptor_set_layout(), set);
        if (!success)
        {
            return false;
        }
        overwrite(set);
        return true;
    }

    void lve_descriptor_writer::overwrite(VkDescriptorSet &set)
    {
        for (auto &write : writes_)
        {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool_.device_.device(), writes_.size(), writes_.data(), 0, nullptr);
    }
} // namespace lve
