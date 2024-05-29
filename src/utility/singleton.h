#pragma once

namespace dae
{
    template <typename T>
    class singleton
    {
    public:
        static T &instance()
        {
            static T instance{};
            return instance;
        }

        virtual ~singleton() = default;
        
        singleton(singleton const &other)            = delete;
        singleton(singleton &&other)                 = delete;
        singleton &operator=(singleton const &other) = delete;
        singleton &operator=(singleton &&other)      = delete;

    protected:
        singleton() = default;
    };
}
