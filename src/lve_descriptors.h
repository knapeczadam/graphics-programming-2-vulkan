#pragma once

#include "lve_device.h"

// Standard includes
#include <memory>
#include <unordered_map>
#include <vector>

namespace lve
{
    class lve_descriptor_set_layout
    {
    public:
        class builder
        {
        public:
            builder(lve_device &device) : device_{device}
            {
            }

            auto add_binding(
                uint32_t           binding,
                VkDescriptorType   descriptor_type,
                VkShaderStageFlags stage_flags,
                uint32_t           count = 1) -> builder &;

            [[nodiscard]] auto build() const -> std::unique_ptr<lve_descriptor_set_layout>;

        private:
            lve_device &device_;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_{};
        };

        lve_descriptor_set_layout(lve_device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~lve_descriptor_set_layout();
        lve_descriptor_set_layout(const lve_descriptor_set_layout &)            = delete;
        lve_descriptor_set_layout &operator=(const lve_descriptor_set_layout &) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptor_set_layout_; }

    private:
        lve_device                   &device_;
        VkDescriptorSetLayout        descriptor_set_layout_;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class lve_descriptor_writer;
    };

    class lve_descriptor_pool
    {
    public:
        class builder
        {
        public:
            builder(lve_device &device) : device_{device}
            {
            }

            auto add_pool_size(VkDescriptorType descriptor_type, uint32_t count) -> builder &;
            auto set_pool_flags(VkDescriptorPoolCreateFlags flags) -> builder &;
            auto set_max_sets(uint32_t count) -> builder &;
            [[nodiscard]] auto build() const -> std::unique_ptr<lve_descriptor_pool>;

        private:
            lve_device                        &device_;
            std::vector<VkDescriptorPoolSize> pool_sizes_{};
            uint32_t                          max_sets_      = 1000;
            VkDescriptorPoolCreateFlags       pool_flags_    = 0;
        };

        lve_descriptor_pool(
            lve_device &device,
            uint32_t max_sets,
            VkDescriptorPoolCreateFlags pool_flags,
            const std::vector<VkDescriptorPoolSize> &pool_sizes);
        ~lve_descriptor_pool();
        lve_descriptor_pool(const lve_descriptor_pool &) = delete;
        lve_descriptor_pool &operator=(const lve_descriptor_pool &) = delete;

        auto allocate_descriptor(const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet &descriptor) const -> bool;
        void free_descriptors(std::vector<VkDescriptorSet> &descriptors) const;

        void reset_pool();

    private:
        lve_device       &device_;
        VkDescriptorPool descriptor_pool_;

        friend class lve_descriptor_writer;
    };

    class lve_descriptor_writer
    {
    public:
        lve_descriptor_writer(lve_descriptor_set_layout &set_layout, lve_descriptor_pool &pool);

        auto write_buffer(uint32_t binding, VkDescriptorBufferInfo *buffer_info) -> lve_descriptor_writer &;
        auto write_image(uint32_t binding, VkDescriptorImageInfo *image_info) -> lve_descriptor_writer &;

        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);

    private:
        lve_descriptor_set_layout         &set_layout_;
        lve_descriptor_pool               &pool_;
        std::vector<VkWriteDescriptorSet> writes_;
    };
} // namespace lve
