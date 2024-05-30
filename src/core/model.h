#pragma once

// Project includes
#include "src/vulkan/buffer.h"

// Standard includes
#include <memory>
#include <string>
#include <vector>

// GLM includes
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace dae
{
    // Forward declarations
    class device;
    
    class model final
    {
    public:
        struct vertex
        {
            glm::vec3 position = {};
            glm::vec3 color    = {};
            glm::vec3 normal   = {};
            glm::vec2 uv       = {};
            glm::vec3 tangent  = {};

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
        
        explicit model(builder const &builder);
        ~model();

        model(model const &)            = delete;
        model &operator=(model const &) = delete;

        static auto create_model(std::string const &file_path) -> std::unique_ptr<model>;
        static auto create_model(std::vector<vertex> const &vertices) -> std::unique_ptr<model>;

        void bind(VkCommandBuffer command_buffer);
        void draw(VkCommandBuffer command_buffer);

    private:
        void create_vertex_buffers(std::vector<vertex> const &vertices);
        void create_index_buffers(std::vector<uint32_t> const &indices);
        

    private:
        device *device_ptr_ = nullptr;
        
        std::unique_ptr<buffer> vertex_buffer_ = nullptr;
        uint32_t                vertex_count_  = 0;

        bool                    has_index_buffer_ = false;
        std::unique_ptr<buffer> index_buffer_     = nullptr;
        uint32_t                index_count_      = 0;
    };
}
