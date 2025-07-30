#pragma once

#include <chrono>

namespace Wow
{
    class State
    {
    public:
        virtual ~State() = default;

        virtual void OnEnter() = 0;
        virtual void OnUpdate( std::chrono::milliseconds dt ) = 0;
        virtual void OnExit() = 0;
    };

    class EmptyState final : public State
    {
    public:
        virtual void OnEnter() override
        {
        }

        virtual void OnUpdate( std::chrono::milliseconds dt ) override
        {
        }

        virtual void OnExit() override
        {
        }
    };
}
