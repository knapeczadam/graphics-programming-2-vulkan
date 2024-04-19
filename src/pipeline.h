#pragma once

// Standard includes
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>


namespace dae
{
    // Forward declarations
    class device;

    struct pipeline_config_info
    {
        pipeline_config_info() = default;
        pipeline_config_info(pipeline_config_info const &) = delete;
        pipeline_config_info &operator=(pipeline_config_info const &) = delete;

        std::vector<VkVertexInputBindingDescription> binding_descriptions{};
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
        VkPipelineViewportStateCreateInfo      viewport_info;
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
        VkPipelineRasterizationStateCreateInfo rasterization_info;
        VkPipelineMultisampleStateCreateInfo   multisample_info;
        VkPipelineColorBlendAttachmentState    color_blend_attachment;
        VkPipelineColorBlendStateCreateInfo    color_blend_info;
        VkPipelineDepthStencilStateCreateInfo  depth_stencil_info;
        std::vector<VkDynamicState>            dynamic_state_enables;
        VkPipelineDynamicStateCreateInfo       dynamic_state_info;
        VkPipelineLayout pipeline_layout = nullptr;
        VkRenderPass     render_pass     = nullptr;
        uint32_t         subpass         = 0;
    };

    class pipeline
    {
    public:
        pipeline() = default;
        pipeline(
            device &device,
            std::string const &vertex_file_path,
            std::string const &fragment_file_path,
            pipeline_config_info const &config_info);

        ~pipeline();

        pipeline(pipeline const &) = delete;
        pipeline &operator=(pipeline const &) = delete;

        void bind(VkCommandBuffer command_buffer);

        static void default_pipeline_config_info(pipeline_config_info &config_info);
        static void enable_alpha_blending(pipeline_config_info &config_info);

    private:
        static auto read_file(std::string const &file_path) -> std::vector<char>;

        void create_graphics_pipeline(
            std::string const &vertex_file_path,
            std::string const &fragment_file_path,
            pipeline_config_info const &config_info);

        void create_shader_module(std::vector<char> const &code, VkShaderModule *shader_module);

        device &device_;
        VkPipeline graphics_pipeline_;
        VkShaderModule vertex_shader_module_;
        VkShaderModule fragment_shader_module_;
    };
}
