#include "scene.h"

// Project includes
#include "src/core/game_object.h"
#include "src/engine/frame_info.h"
#include "src/system/i_system.h"

// Standard includes
#include <algorithm>
#include <ranges>

namespace dae
{
    scene::scene() = default;

    scene::scene(std::string name, std::unique_ptr<i_system> system)
        : name_(std::move(name))
          , system_{std::move(system)}
    {
    }

    scene::~scene() = default;

    void scene::update()
    {
        auto &frame = frame_info::instance();
        frame.game_objects = objects();
        system_->update();
    }

    void scene::render() const
    {
        auto &frame = frame_info::instance();
        frame.game_objects = objects();
        system_->render();
    }

    auto scene::create_game_object(std::string const &name) -> game_object *
    {
        objects_.emplace_back(std::make_unique<game_object>(name));
        return objects_.back().get();
    }
    
    auto scene::objects() const -> std::vector<game_object*>
    {
        std::vector<game_object*> objects;
        objects.reserve(objects_.size());
        for (auto const &object : objects_)
        {
            objects.push_back(object.get());
        }
        return objects;
    }
}
