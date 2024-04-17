#pragma once

// Project includes
#include "device.h"
#include "buffer.h"

// GLM includes
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// Standard includes
#include <vector>
#include <memory>

namespace dae
{
    class model
    {
    public:
        struct vertex
        {
            glm::vec3 position = {};
            glm::vec3 color    = {};
            glm::vec3 normal   = {};
            glm::vec2 uv       = {};

            static auto get_binding_description() -> std::vector<VkVertexInputBindingDescription>;
            static auto get_attribute_descriptions() -> std::vector<VkVertexInputAttributeDescription>;

            bool operator==(vertex const &other) const;
        };

        struct builder
        {
            std::vector<vertex> vertices = {};
            std::vector<uint32_t>   indices = {};

            void load_model(std::string const &file_path);
        };
        
        model(device &device, builder const &builder);
        ~model();

        model(model const &)            = delete;
        model &operator=(model const &) = delete;

        static auto create_model_from_file(device &device, std::string const &file_path) -> std::unique_ptr<model>;
        static auto create_model_from_vertices(device &device, std::vector<vertex> const &vertices) -> std::unique_ptr<model>;

        void bind(VkCommandBuffer command_buffer);
        void draw(VkCommandBuffer command_buffer);

    private:
        void create_vertex_buffers(std::vector<vertex> const &vertices);
        void create_index_buffers(std::vector<uint32_t> const &indices);
        

    private:
        device     &device_;
        
        std::unique_ptr<buffer> vertex_buffer_;
        uint32_t                    vertex_count_;

        bool                        has_index_buffer_ = false;
        std::unique_ptr<buffer> index_buffer_;
        uint32_t                    index_count_;
    };
}
