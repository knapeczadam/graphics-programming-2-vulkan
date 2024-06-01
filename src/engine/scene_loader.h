#pragma once

// Project includes
#include "src/engine/engine.h"
#include "src/utility/singleton.h"

// Standard includes
#include <string>

namespace dae
{
    class scene_loader final : public singleton<scene_loader>
    {
    public:
        ~scene_loader() override = default;

        scene_loader(scene_loader const& other)            = delete;
        scene_loader(scene_loader &&other)                 = delete;
        scene_loader &operator=(scene_loader const &other) = delete;
        scene_loader &operator=(scene_loader &&other)      = delete;

        void load_scenes();
        void load_2d_scene();
        void load_3d_scene();
        void load_light_scene();
        void load_material_pbr_scene();
        void load_texture_pbr_scene();

        [[nodiscard]] auto texture_path() const -> std::string const & { return texture_path_; }
        [[nodiscard]] auto diffuse_texture_path() const -> std::string const & { return diffuse_texture_path_; }
        [[nodiscard]] auto normal_texture_path() const -> std::string const & { return normal_texture_path_; }
        [[nodiscard]] auto specular_texture_path() const -> std::string const & { return specular_texture_path_; }
        [[nodiscard]] auto glossiness_texture_path() const -> std::string const & { return glossiness_texture_path_; }

    private:
        friend class singleton<scene_loader>;
        scene_loader() = default;

    private:
        std::string const debug_texture_path_ = engine::data_path + "assets/textures/debug.png";
        std::string texture_path_             = debug_texture_path_;
        std::string diffuse_texture_path_     = debug_texture_path_;
        std::string normal_texture_path_      = debug_texture_path_;
        std::string specular_texture_path_    = debug_texture_path_;
        std::string glossiness_texture_path_  = debug_texture_path_;
    };
}
