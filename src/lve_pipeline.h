#pragma once

// Standard includes
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>


namespace lve
{
    // Forward declarations
    class lve_device;

    struct pipeline_config_info
    {
        pipeline_config_info() = default;
        pipeline_config_info(pipeline_config_info const &) = delete;
        pipeline_config_info &operator=(pipeline_config_info const &) = delete;

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

    class lve_pipeline
    {
    public:
        lve_pipeline() = default;
        lve_pipeline(
            lve_device &device,
            std::string const &vertex_file_path,
            std::string const &fragment_file_path,
            pipeline_config_info const &config_info);

        ~lve_pipeline();

        lve_pipeline(lve_pipeline const &) = delete;
        lve_pipeline &operator=(lve_pipeline const &) = delete;

        void bind(VkCommandBuffer command_buffer);

        static void default_pipeline_config_info(pipeline_config_info &config_info);

    private:
        static auto read_file(std::string const &file_path) -> std::vector<char>;

        void create_graphics_pipeline(
            std::string const &vertex_file_path,
            std::string const &fragment_file_path,
            pipeline_config_info const &config_info);

        void create_shader_module(std::vector<char> const &code, VkShaderModule *shader_module);

        lve_device &device_;
        VkPipeline graphics_pipeline_;
        VkShaderModule vertex_shader_module_;
        VkShaderModule fragment_shader_module_;
    };
}
