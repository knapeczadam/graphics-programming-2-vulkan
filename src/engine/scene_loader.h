#pragma once

// Project includes
#include "src/utility/singleton.h"

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
        void load_pbr_scene();

    private:
        friend class singleton<scene_loader>;
        scene_loader() = default;
    };
}
