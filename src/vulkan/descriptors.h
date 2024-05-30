#pragma once

// Standard includes
#include <memory>
#include <unordered_map>
#include <vector>

// Vulkan includes
#include <vulkan/vulkan.h>

namespace dae
{
    // Forward declarations
    class device;
    
    class descriptor_set_layout final
    {
    public:
        class builder
        {
        public:
            builder();

            auto add_binding(
                uint32_t           binding,
                VkDescriptorType   descriptor_type,
                VkShaderStageFlags stage_flags,
                uint32_t           count = 1) -> builder &;

            [[nodiscard]] auto build() const -> std::unique_ptr<descriptor_set_layout>;

        private:
            device *device_ptr_ = nullptr;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_{};
        };

        explicit descriptor_set_layout(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~descriptor_set_layout();
        
        descriptor_set_layout(descriptor_set_layout const &other)            = delete;
        descriptor_set_layout &operator=(descriptor_set_layout const &other) = delete;

        [[nodiscard]] auto get_descriptor_set_layout() const -> VkDescriptorSetLayout { return descriptor_set_layout_; }

    private:
        device                *device_ptr_           = nullptr;
        VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_ = {};

        friend class descriptor_writer;
    };

    class descriptor_pool final
    {
    public:
        class builder
        {
        public:
            builder();

            auto add_pool_size(VkDescriptorType descriptor_type, uint32_t count) -> builder &;
            auto set_pool_flags(VkDescriptorPoolCreateFlags flags) -> builder &;
            auto set_max_sets(uint32_t count) -> builder &;
            [[nodiscard]] auto build() const -> std::unique_ptr<descriptor_pool>;

        private:
            device                            *device_ptr_ = nullptr;
            std::vector<VkDescriptorPoolSize> pool_sizes_  = {};
            uint32_t                          max_sets_    = 1000;
            VkDescriptorPoolCreateFlags       pool_flags_  = 0;
        };

        descriptor_pool(
            uint32_t max_sets,
            VkDescriptorPoolCreateFlags pool_flags,
            const std::vector<VkDescriptorPoolSize> &pool_sizes);
        ~descriptor_pool();
        
        descriptor_pool(const descriptor_pool &other)            = delete;
        descriptor_pool &operator=(const descriptor_pool &other) = delete;

        auto allocate_descriptor(const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet &descriptor) const -> bool;
        void free_descriptors(std::vector<VkDescriptorSet> &descriptors) const;

        void reset_pool();

    private:
        device           *device_ptr_     = nullptr;
        VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;

        friend class descriptor_writer;
    };

    class descriptor_writer final
    {
    public:
        descriptor_writer(descriptor_set_layout *set_layout_ptr, descriptor_pool *pool_ptr);

        auto write_buffer(uint32_t binding, VkDescriptorBufferInfo *buffer_info) -> descriptor_writer &;
        auto write_image(uint32_t binding, VkDescriptorImageInfo *image_info) -> descriptor_writer &;

        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);

    private:
        descriptor_set_layout             *set_layout_ptr_ = nullptr;
        descriptor_pool                   *pool_ptr_       = nullptr;
        std::vector<VkWriteDescriptorSet> writes_          = {};
    };
}
