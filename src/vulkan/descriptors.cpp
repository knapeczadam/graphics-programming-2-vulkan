#include "descriptors.h"

// Project includes
#include "src/vulkan/device.h"

// Standard includes
#include <cassert>
#include <stdexcept>

namespace dae
{
    //--------------------------------------------------------------------------------------------------
    // Descriptor Set Layout Builder
    //--------------------------------------------------------------------------------------------------
    descriptor_set_layout::builder::builder()
        : device_ptr_{&device::instance()}
    {
    }

    descriptor_set_layout::builder &descriptor_set_layout::builder::add_binding(
        uint32_t binding,
        VkDescriptorType descriptor_type,
        VkShaderStageFlags stage_flags,
        uint32_t count)
    {
        assert(bindings_.count(binding) == 0 and "Binding already in use");
        VkDescriptorSetLayoutBinding layout_binding{};
        layout_binding.binding         = binding;
        layout_binding.descriptorType  = descriptor_type;
        layout_binding.descriptorCount = count;
        layout_binding.stageFlags      = stage_flags;
        bindings_[binding]            = layout_binding;
        return *this;
    }

    auto descriptor_set_layout::builder::build() const -> std::unique_ptr<descriptor_set_layout>
    {
        return std::make_unique<descriptor_set_layout>(bindings_);
    }

    //--------------------------------------------------------------------------------------------------
    // Descriptor Set Layout
    //--------------------------------------------------------------------------------------------------
    descriptor_set_layout::descriptor_set_layout(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : device_ptr_{&device::instance()}
        , bindings_{bindings}
    {
        std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};
        set_layout_bindings.reserve(bindings.size());
        for (auto const &[fst, snd] : bindings)
        {
            set_layout_bindings.push_back(snd);
        }

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
        descriptor_set_layout_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
        descriptor_set_layout_info.pBindings    = set_layout_bindings.data();

        if (vkCreateDescriptorSetLayout(
            device_ptr_->logical_device(),
            &descriptor_set_layout_info,
            nullptr,
            &descriptor_set_layout_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    descriptor_set_layout::~descriptor_set_layout()
    {
        vkDestroyDescriptorSetLayout(device_ptr_->logical_device(), descriptor_set_layout_, nullptr);
    }

    //--------------------------------------------------------------------------------------------------
    // Descriptor Pool Builder
    //--------------------------------------------------------------------------------------------------
    descriptor_pool::builder::builder()
        : device_ptr_{&device::instance()}
    {
    }

    auto descriptor_pool::builder::add_pool_size(VkDescriptorType const descriptor_type, uint32_t count) -> descriptor_pool::builder &
    {
        pool_sizes_.push_back({descriptor_type, count});
        return *this;
    }

    auto descriptor_pool::builder::set_pool_flags(VkDescriptorPoolCreateFlags flags) -> descriptor_pool::builder &
    {
        pool_flags_ = flags;
        return *this;
    }

    auto descriptor_pool::builder::set_max_sets(uint32_t count) -> descriptor_pool::builder &
    {
        max_sets_ = count;
        return *this;
    }

    auto descriptor_pool::builder::build() const -> std::unique_ptr<descriptor_pool>
    {
        return std::make_unique<descriptor_pool>(max_sets_, pool_flags_, pool_sizes_);
    }

    //--------------------------------------------------------------------------------------------------
    // Descriptor Pool
    //--------------------------------------------------------------------------------------------------
    descriptor_pool::descriptor_pool(
        uint32_t max_sets,
        VkDescriptorPoolCreateFlags pool_flags,
        const std::vector<VkDescriptorPoolSize> &pool_sizes)
        : device_ptr_{&device::instance()}
    {
        VkDescriptorPoolCreateInfo descriptor_pool_info{};
        descriptor_pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        descriptor_pool_info.pPoolSizes    = pool_sizes.data();
        descriptor_pool_info.maxSets       = max_sets;
        descriptor_pool_info.flags         = pool_flags;

        if (vkCreateDescriptorPool(device_ptr_->logical_device(), &descriptor_pool_info, nullptr, &descriptor_pool_) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    descriptor_pool::~descriptor_pool()
    {
        vkDestroyDescriptorPool(device_ptr_->logical_device(), descriptor_pool_, nullptr);
    }

    auto descriptor_pool::allocate_descriptor(const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet &descriptor) const -> bool
    {
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool     = descriptor_pool_;
        alloc_info.pSetLayouts        = &descriptor_set_layout;
        alloc_info.descriptorSetCount = 1;

        if (vkAllocateDescriptorSets(device_ptr_->logical_device(), &alloc_info, &descriptor) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }

    void descriptor_pool::free_descriptors(std::vector<VkDescriptorSet> &descriptors) const
    {
        vkFreeDescriptorSets(
            device_ptr_->logical_device(),
            descriptor_pool_,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void descriptor_pool::reset_pool()
    {
        vkResetDescriptorPool(device_ptr_->logical_device(), descriptor_pool_, 0);
    }

    //--------------------------------------------------------------------------------------------------
    // Descriptor Writer
    //--------------------------------------------------------------------------------------------------
    descriptor_writer::descriptor_writer(descriptor_set_layout *set_layout_ptr, descriptor_pool *pool_ptr)
        : set_layout_ptr_{set_layout_ptr}, pool_ptr_{pool_ptr}
    {
    }

    auto descriptor_writer::write_buffer(uint32_t binding, VkDescriptorBufferInfo *buffer_info) -> descriptor_writer &
    {
        assert(set_layout_ptr_->bindings_.count(binding) == 1 and "Layout does not contain specified binding");

        auto &binding_description = set_layout_ptr_->bindings_[binding];

        assert(binding_description.descriptorCount == 1 and "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType  = binding_description.descriptorType;
        write.dstBinding      = binding;
        write.pBufferInfo     = buffer_info;
        write.descriptorCount = 1;

        writes_.push_back(write);
        return *this;
    }

    auto descriptor_writer::write_image(uint32_t binding, VkDescriptorImageInfo *image_info) -> descriptor_writer &
    {
        assert(set_layout_ptr_->bindings_.count(binding) == 1 and "Layout does not contain specified binding");

        auto &binding_description = set_layout_ptr_->bindings_[binding];

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

    auto descriptor_writer::build(VkDescriptorSet &set) -> bool
    {
        bool success = pool_ptr_->allocate_descriptor(set_layout_ptr_->get_descriptor_set_layout(), set);
        if (not success)
        {
            return false;
        }
        overwrite(set);
        return true;
    }

    void descriptor_writer::overwrite(VkDescriptorSet &set)
    {
        for (auto &write : writes_)
        {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool_ptr_->device_ptr_->logical_device(), static_cast<uint32_t>(writes_.size()), writes_.data(), 0, nullptr);
    }
}
