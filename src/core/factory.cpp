#include "factory.h"

// Standard includes
#include <iostream>

// GLM includes
#include <glm/gtc/constants.hpp>

namespace dae
{
    std::unique_ptr<model> factory::create_oval(glm::vec3 offset, float radiusX, float radiusY, int segments)
    {
        model::builder modelBuilder{};

        // Generate vertices for the oval shape
        for (int i = 0; i <= segments; ++i)
        {
            float angle = glm::two_pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
            float x = radiusX * std::cos(angle);
            float y = radiusY * std::sin(angle);
            float r = std::rand() % 101 / 100.0f;
            float g = std::rand() % 101 / 100.0f;
            float b = std::rand() % 101 / 100.0f;
            modelBuilder.vertices.push_back({{x + offset.x, y + offset.y, offset.z}, {r, g, b}});
        }

        // Generate indices for the triangles forming the oval shape
        for (int i = 1; i < segments; ++i)
        {
            modelBuilder.indices.push_back(0);
            modelBuilder.indices.push_back(i);
            modelBuilder.indices.push_back(i + 1);
        }

        // Close the oval by connecting the last and first vertices
        modelBuilder.indices.push_back(0);
        modelBuilder.indices.push_back(segments);
        modelBuilder.indices.push_back(1);

        return std::make_unique<model>(modelBuilder);
    }

    std::unique_ptr<model> factory::create_n_gon(glm::vec3 offset, float radius, int sides)
    {
        model::builder modelBuilder{};

        // Calculate the angle between each vertex based on the number of sides
        float angleIncrement = glm::two_pi<float>() / static_cast<float>(sides);

        // Generate vertices for the n-gon shape
        for (int i = 0; i < sides; ++i)
        {
            float angle = angleIncrement * static_cast<float>(i);
            float x = radius * std::cos(angle);
            float y = radius * std::sin(angle);
            float r = std::rand() % 101 / 100.0f;
            float g = std::rand() % 101 / 100.0f;
            float b = std::rand() % 101 / 100.0f;
            modelBuilder.vertices.push_back({{x + offset.x, y + offset.y, offset.z}, {r, g, b}});
        }

        // Generate indices for the triangles forming the n-gon shape
        for (int i = 1; i < sides - 1; ++i)
        {
            modelBuilder.indices.push_back(0);
            modelBuilder.indices.push_back(i);
            modelBuilder.indices.push_back(i + 1);
        }

        // Close the n-gon by connecting the last and first vertices
        modelBuilder.indices.push_back(0);
        modelBuilder.indices.push_back(sides - 1);
        modelBuilder.indices.push_back(1);

        return std::make_unique<model>(modelBuilder);
    }
}
