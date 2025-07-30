#pragma once

#include "client/states/State.hpp"

#include <variant>

namespace Wow
{
    namespace Detail
    {
        template<typename VariantType, typename T, size_t index = 0>
        constexpr size_t IndexOf()
        {
            if constexpr ( index == std::variant_size_v<VariantType> )
            {
                return index;
            }
            else if constexpr ( std::is_same_v<std::variant_alternative_t<index, VariantType>, T> )
            {
                return index;
            }
            else
            {
                return IndexOf<VariantType, T, index + 1>();
            }
        }
    }

    template< typename ...States>
    class StateMachine
    {
    public:
        template< typename State, typename ...Args >
        void ChangeState( Args && ... args)
        {
            static_assert( std::is_base_of_v< Wow::State, State > );

            if ( m_states.index() == Detail::IndexOf< decltype( m_states ), State >() )
                return;

            std::visit( []( auto & state )
            {
                state.OnExit();
            }, m_states );

            m_states = State{ std::forward< Args >( args )... };

            std::visit( []( auto & state )
            {
                state.OnEnter();
            }, m_states );
        }

        void Update( std::chrono::milliseconds dt )
        {
            std::visit( [dt]( auto & state )
            {
                state.OnUpdate( dt );
            }, m_states );
        }

    private:
        std::variant < States... > m_states;
    };
}
