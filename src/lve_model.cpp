#include "lve_model.h"

// Standard includes
#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>

// TOL includes
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

// Project includes
#include "lve_utils.h"

// GLM includes
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std
{
    template<>
    struct hash<lve::lve_model::lve_vertex>
    {
        auto operator()(lve::lve_model::lve_vertex const &vertex) const noexcept -> size_t
        {
            size_t seed = 0;
            lve::hash_combine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
    
}


namespace lve
{
    auto lve_model::lve_vertex::get_binding_description() -> std::vector<VkVertexInputBindingDescription>
    {
        std::vector<VkVertexInputBindingDescription> binding_description(1);
        binding_description[0].binding   = 0;
        binding_description[0].stride    = sizeof(lve_vertex);
        binding_description[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding_description;
    }

    auto lve_model::lve_vertex::get_attribute_descriptions() -> std::vector<VkVertexInputAttributeDescription>
    {
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
        VkVertexInputAttributeDescription position{
            .location = 0,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = offsetof(lve_vertex, position)
        };
        attribute_descriptions.push_back(position);

        VkVertexInputAttributeDescription color{
            .location = 1,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = offsetof(lve_vertex, color)
        };
        attribute_descriptions.push_back(color);
        
        VkVertexInputAttributeDescription normal{
            .location = 2,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = offsetof(lve_vertex, normal)
        };
        attribute_descriptions.push_back(normal);
        
        VkVertexInputAttributeDescription uv{
            .location = 3,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32_SFLOAT,
            .offset   = offsetof(lve_vertex, uv)
        };
        attribute_descriptions.push_back(uv);
        
        return attribute_descriptions;
    }

    bool lve_model::lve_vertex::operator==(lve_vertex const &other) const
    {
        return position == other.position and color == other.color and normal == other.normal and uv == other.uv;
    }

    void lve_model::lve_builder::load_model(std::string const &file_path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (not tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file_path.c_str()))
        {
            throw std::runtime_error{warn + err};
        }

        vertices.clear();
        indicies.clear();

        std::unordered_map<lve_vertex, uint32_t> unique_vertices{};

        for (auto const &shape : shapes)
        {
            for (auto const &index : shape.mesh.indices)
            {
                lve_vertex vertex{};

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
                        attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                if (not unique_vertices.contains(vertex))
                {
                    unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indicies.push_back(unique_vertices[vertex]);
            }
        }
    }

    lve_model::lve_model(lve_device &device, lve_builder const &builder)
        : device_{device}
    {
        create_vertex_buffers(builder.vertices);
        create_index_buffers(builder.indicies);
    }

    lve_model::~lve_model()
    {
    }

    auto lve_model::create_model_from_file(lve_device &device, std::string const &file_path) -> std::unique_ptr<lve_model>
    {
        lve_builder builder{};
        builder.load_model(file_path);
        std::cout << "Vertex count: " << builder.vertices.size() << '\n';
        return std::make_unique<lve_model>(device, builder);
    }

    void lve_model::bind(VkCommandBuffer command_buffer)
    {
        VkBuffer     buffers[] = {vertex_buffer_->get_buffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

        if (has_index_buffer_)
        {
            vkCmdBindIndexBuffer(command_buffer, index_buffer_->get_buffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void lve_model::draw(VkCommandBuffer command_buffer)
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

    void lve_model::create_vertex_buffers(std::vector<lve_vertex> const &vertices)
    {
        vertex_count_ = static_cast<uint32_t>(vertices.size());
        assert(vertex_count_ >= 3 and "Vertex count must be at least 3!");
        VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;
        uint32_t vertex_size = sizeof(vertices[0]);

        lve_buffer staging_buffer {
            device_,
            vertex_size,
            vertex_count_,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        staging_buffer.map();
        staging_buffer.write_to_buffer((void*) vertices.data());

        vertex_buffer_ = std::make_unique<lve_buffer>(
            device_,
            vertex_size,
            vertex_count_,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );


        device_.copy_buffer(staging_buffer.get_buffer(), vertex_buffer_->get_buffer(), buffer_size);
    }

    void lve_model::create_index_buffers(std::vector<uint32_t> const& indices)
    {
        index_count_ = static_cast<uint32_t>(indices.size());
        has_index_buffer_ = index_count_ > 0;

        if (not has_index_buffer_)
        {
            return;
        }
        
        VkDeviceSize buffer_size = sizeof(indices[0]) * index_count_;
        uint32_t index_size = sizeof(indices[0]);

        lve_buffer staging_buffer {
            device_,
            index_size,
            index_count_,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        staging_buffer.map();
        staging_buffer.write_to_buffer((void*) indices.data());

        index_buffer_ = std::make_unique<lve_buffer>(
            device_,
            index_size,
            index_count_,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        device_.copy_buffer(staging_buffer.get_buffer(), index_buffer_->get_buffer(), buffer_size);
    }
}
