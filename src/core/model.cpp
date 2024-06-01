#include "model.h"

// Project includes
#include "src/engine/engine.h"
#include "src/utility/utils.h"
#include "src/vulkan/device.h"

// Standard includes
#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>

// TOL includes
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

// GLM includes
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#if defined(CMAKE_BUILD)
#ifndef ENGINE_DIR
#define ENGINE_DIR "../../../"
#endif
#else
#ifndef ENGINE_DIR
#define ENGINE_DIR ""
#endif
#endif

namespace std
{
    template<>
    struct hash<dae::model::vertex>
    {
        auto operator()(dae::model::vertex const &vertex) const noexcept -> size_t
        {
            size_t seed = 0;
            dae::hash_combine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
    
}


namespace dae
{
    auto model::vertex::get_binding_description() -> std::vector<VkVertexInputBindingDescription>
    {
        std::vector<VkVertexInputBindingDescription> binding_description(1);
        binding_description[0].binding   = 0;
        binding_description[0].stride    = sizeof(vertex);
        binding_description[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding_description;
    }

    auto model::vertex::get_attribute_descriptions() -> std::vector<VkVertexInputAttributeDescription>
    {
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
        VkVertexInputAttributeDescription position{
            .location = 0,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = offsetof(vertex, position)
        };
        attribute_descriptions.push_back(position);

        VkVertexInputAttributeDescription color{
            .location = 1,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = offsetof(vertex, color)
        };
        attribute_descriptions.push_back(color);
        
        VkVertexInputAttributeDescription normal{
            .location = 2,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = offsetof(vertex, normal)
        };
        attribute_descriptions.push_back(normal);
        
        VkVertexInputAttributeDescription uv{
            .location = 3,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32_SFLOAT,
            .offset   = offsetof(vertex, uv)
        };
        attribute_descriptions.push_back(uv);

        VkVertexInputAttributeDescription tangent{
            .location = 4,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = offsetof(vertex, tangent)
        };
        attribute_descriptions.push_back(tangent);
        
        return attribute_descriptions;
    }

    bool model::vertex::operator==(vertex const &other) const
    {
        return position == other.position and color == other.color and normal == other.normal and uv == other.uv;
    }

    void model::builder::load_model(std::string const &file_path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string const path = ENGINE_DIR + engine::data_path + file_path;
        if (not tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
        {
            throw std::runtime_error{warn + err};
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<vertex, uint32_t> unique_vertices{};

        for (auto const &shape : shapes)
        {
            for (auto const &index : shape.mesh.indices)
            {
                vertex vertex{};

                if (index.vertex_index >= 0)
                {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2]
                    };
                }
                
                if (index.normal_index >= 0)
                {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }
                
                if (index.texcoord_index >= 0)
                {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                if (not unique_vertices.contains(vertex))
                {
                    unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(unique_vertices[vertex]);
            }

            // Cheap tangent calculation
            for (size_t i = 0; i < indices.size(); i += 3)
            {
                vertex &v0 = vertices[indices[i + 0]];
                vertex &v1 = vertices[indices[i + 1]];
                vertex &v2 = vertices[indices[i + 2]];
            
                glm::vec3 edge1 = v1.position - v0.position;
                glm::vec3 edge2 = v2.position - v0.position;
            
                glm::vec2 uv0 = v0.uv;
                glm::vec2 uv1 = v1.uv;
                glm::vec2 uv2 = v2.uv;
            
                glm::vec2 delta_uv1 = uv1 - uv0;
                glm::vec2 delta_uv2 = uv2 - uv0;

                float f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);

                glm::vec3 tangent = f * (delta_uv2.y * edge1 - delta_uv1.y * edge2);
                tangent = glm::normalize(tangent);

                float sx = delta_uv1.x;
                float sy = delta_uv2.x;
                float tx = delta_uv1.y;
                float ty = delta_uv2.y;
                float handedness = (sx * ty - sy * tx) > 0.0f ? 1.0f : -1.0f;
                tangent *= handedness;
                v0.tangent = tangent;
                v1.tangent = tangent;
                v2.tangent = tangent;
            }
        }
    }

    model::model(builder const &builder)
        : device_ptr_{&device::instance()}
    {
        create_vertex_buffers(builder.vertices);
        create_index_buffers(builder.indices);
    }

    model::~model() = default;

    auto model::create_model(std::string const &file_path) -> std::unique_ptr<model>
    {
        builder builder{};
        builder.load_model(file_path);
        std::cout << "Vertex count: " << builder.vertices.size() << '\n';
        return std::make_unique<model>(builder);
    }

    auto model::create_model(std::vector<vertex> const &vertices) -> std::unique_ptr<model>
    {
        builder builder{};
        builder.vertices = vertices;
        std::cout << "Vertex count: " << builder.vertices.size() << '\n';
        return std::make_unique<model>(builder);
    }

    void model::bind(VkCommandBuffer command_buffer)
    {
        VkBuffer     buffers[] = {vertex_buffer_->get_buffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

        if (has_index_buffer_)
        {
            vkCmdBindIndexBuffer(command_buffer, index_buffer_->get_buffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void model::draw(VkCommandBuffer command_buffer)
    {
        if (has_index_buffer_)
        {
            vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);
        }
        else
        {
            vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
        }
    }

    void model::create_vertex_buffers(std::vector<vertex> const &vertices)
    {
        vertex_count_ = static_cast<uint32_t>(vertices.size());
        assert(vertex_count_ >= 3 and "Vertex count must be at least 3!");
        VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;
        uint32_t vertex_size = sizeof(vertices[0]);

        buffer staging_buffer {
            vertex_size,
            vertex_count_,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        staging_buffer.map();
        staging_buffer.write_to_buffer(const_cast<vertex*>(vertices.data()));

        vertex_buffer_ = std::make_unique<buffer>(
            vertex_size,
            vertex_count_,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );


        device_ptr_->copy_buffer(staging_buffer.get_buffer(), vertex_buffer_->get_buffer(), buffer_size);
    }

    void model::create_index_buffers(std::vector<uint32_t> const& indices)
    {
        index_count_ = static_cast<uint32_t>(indices.size());
        has_index_buffer_ = index_count_ > 0;

        if (not has_index_buffer_)
        {
            return;
        }
        
        VkDeviceSize buffer_size = sizeof(indices[0]) * index_count_;
        uint32_t index_size = sizeof(indices[0]);

        buffer staging_buffer {
            index_size,
            index_count_,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        staging_buffer.map();
        staging_buffer.write_to_buffer((void*) indices.data());

        index_buffer_ = std::make_unique<buffer>(
            index_size,
            index_count_,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        device_ptr_->copy_buffer(staging_buffer.get_buffer(), index_buffer_->get_buffer(), buffer_size);
    }
}
