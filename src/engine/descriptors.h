#pragma once

// Project includes
#include "src/engine/device.h"

// Standard includes
#include <memory>
#include <unordered_map>
#include <vector>

namespace dae
{
    class descriptor_set_layout
    {
    public:
        class builder
        {
        public:
            builder(device &device) : device_{device}
            {
            }

            auto add_binding(
                uint32_t           binding,
                VkDescriptorType   descriptor_type,
                VkShaderStageFlags stage_flags,
                uint32_t           count = 1) -> builder &;

            [[nodiscard]] auto build() const -> std::unique_ptr<descriptor_set_layout>;

        private:
            device &device_;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_{};
        };

        descriptor_set_layout(device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~descriptor_set_layout();
        descriptor_set_layout(descriptor_set_layout const &)            = delete;
        descriptor_set_layout &operator=(descriptor_set_layout const &) = delete;

        [[nodiscard]] auto get_descriptor_set_layout() const -> VkDescriptorSetLayout { return descriptor_set_layout_; }

    private:
        device                       &device_;
        VkDescriptorSetLayout        descriptor_set_layout_;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class descriptor_writer;
    };

    class descriptor_pool
    {
    public:
        class builder
        {
        public:
            builder(device &device) : device_{device}
            {
            }

            auto add_pool_size(VkDescriptorType descriptor_type, uint32_t count) -> builder &;
            auto set_pool_flags(VkDescriptorPoolCreateFlags flags) -> builder &;
            auto set_max_sets(uint32_t count) -> builder &;
            [[nodiscard]] auto build() const -> std::unique_ptr<descriptor_pool>;

        private:
            device                        &device_;
            std::vector<VkDescriptorPoolSize> pool_sizes_{};
            uint32_t                          max_sets_      = 1000;
            VkDescriptorPoolCreateFlags       pool_flags_    = 0;
        };

        descriptor_pool(
            device &device,
            uint32_t max_sets,
            VkDescriptorPoolCreateFlags pool_flags,
            const std::vector<VkDescriptorPoolSize> &pool_sizes);
        ~descriptor_pool();
        descriptor_pool(const descriptor_pool &) = delete;
        descriptor_pool &operator=(const descriptor_pool &) = delete;

        auto allocate_descriptor(const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet &descriptor) const -> bool;
        void free_descriptors(std::vector<VkDescriptorSet> &descriptors) const;

        void reset_pool();

    private:
        device       &device_;
        VkDescriptorPool descriptor_pool_;

        friend class descriptor_writer;
    };

    class descriptor_writer
    {
    public:
        descriptor_writer(descriptor_set_layout &set_layout, descriptor_pool &pool);

        auto write_buffer(uint32_t binding, VkDescriptorBufferInfo *buffer_info) -> descriptor_writer &;
        auto write_image(uint32_t binding, VkDescriptorImageInfo *image_info) -> descriptor_writer &;

        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);

    private:
        descriptor_set_layout         &set_layout_;
        descriptor_pool               &pool_;
        std::vector<VkWriteDescriptorSet> writes_;
    };
}
