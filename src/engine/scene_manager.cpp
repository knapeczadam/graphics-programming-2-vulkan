#include "scene_manager.h"

// Project includes
#include "src/engine/scene.h"

// Standard includes
#include <ranges>

namespace dae
{
    scene_manager::scene_manager() = default;

    scene_manager::~scene_manager() = default;

    void scene_manager::update()
    {
        for (auto const &scene : scenes_)
        {
            scene->update();
        }
    }

    void scene_manager::render()
    {
        for (auto const &scene : scenes_)
        {
            scene->render();
        }
    }

    auto scene_manager::find(std::string const &name) -> scene *
    {
        auto const it = std::ranges::find_if(scenes_, [&name](auto const &scene)
        {
            return scene->name() == name;
        });
        return it != scenes_.end() ? it->get() : nullptr;
    }

    auto scene_manager::create_scene(std::string const &name, std::unique_ptr<i_system> system) -> scene *
    {
        scenes_.emplace_back(std::unique_ptr<scene>(new scene(name, std::move(system))));
        return scenes_.back().get();
    }
}
