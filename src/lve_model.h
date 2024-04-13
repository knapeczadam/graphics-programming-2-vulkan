#pragma once

// Project includes
#include "lve_device.h"
#include "lve_buffer.h"

// GLM includes
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// Standard includes
#include <vector>
#include <memory>

namespace lve
{
    class lve_model
    {
    public:
        struct lve_vertex
        {
            glm::vec3 position = {};
            glm::vec3 color    = {};
            glm::vec3 normal   = {};
            glm::vec2 uv       = {};

            static auto get_binding_description() -> std::vector<VkVertexInputBindingDescription>;
            static auto get_attribute_descriptions() -> std::vector<VkVertexInputAttributeDescription>;

            bool operator==(lve_vertex const &other) const;
        };

        struct lve_builder
        {
            std::vector<lve_vertex> vertices = {};
            std::vector<uint32_t>   indicies = {};

            void load_model(std::string const &file_path);
        };
        
        lve_model(lve_device &device, lve_builder const &builder);
        ~lve_model();

        lve_model(lve_model const &)            = delete;
        lve_model &operator=(lve_model const &) = delete;

        static auto create_model_from_file(lve_device &device, std::string const &file_path) -> std::unique_ptr<lve_model>;

        void bind(VkCommandBuffer command_buffer);
        void draw(VkCommandBuffer command_buffer);

    private:
        void create_vertex_buffers(std::vector<lve_vertex> const &vertices);
        void create_index_buffers(std::vector<uint32_t> const &indices);
        

    private:
        lve_device     &device_;
        
        std::unique_ptr<lve_buffer> vertex_buffer_;
        uint32_t                    vertex_count_;

        bool                        has_index_buffer_ = false;
        std::unique_ptr<lve_buffer> index_buffer_;
        uint32_t                    index_count_;
    };
}
