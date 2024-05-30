#pragma once

// Project includes
#include "src/utility/singleton.h"

namespace dae
{
    class game_time final : public singleton<game_time>
    {
    public:
        ~game_time() override = default;

        game_time(game_time const &other)            = delete;
        game_time(game_time &&other)                 = delete;
        game_time &operator=(game_time const &other) = delete;
        game_time &operator=(game_time &&other)      = delete;

        [[nodiscard]] auto delta_time() const -> float { return delta_time_; }
        [[nodiscard]] auto fixed_delta_time() const -> float { return fixed_delta_time_; }
        [[nodiscard]] auto ms_per_frame() const -> float { return ms_per_frame_; }

        void set_delta_time(float delta_time) { delta_time_ = delta_time; }
        
    private:
        friend class singleton<game_time>;
        game_time() = default;
        
    private:
        float delta_time_ = 0.0f;
        float const fixed_delta_time_ = 0.02f;
        float const ms_per_frame_ = 0.0f;
        
    };
}
