#include "client/GameClient.hpp"
#include "auth/AuthSession.hpp"
#include "game/GameSession.hpp"

#include <boost/asio/signal_set.hpp>
#include <charconv>
#include <iostream>

namespace Wow
{
    constexpr uint16_t DEFAULT_REALM_PORT = 3724u;

    static boost::asio::ip::tcp::endpoint ParseEndpoint( boost::asio::io_context & context, std::string_view address )
    {
        std::string partAddress = { address.data(), address.size() };
        std::string partPort = std::to_string( DEFAULT_REALM_PORT );

        const auto pos = address.find_last_of( ":" );
        if ( pos != std::string::npos )
        {
            partAddress = address.substr( 0, pos );
            partPort = address.substr( pos + 1 );
        }

        boost::asio::ip::tcp::resolver resolver( context );
        boost::system::error_code ec{};
        auto basic_result = resolver.resolve(partAddress, partPort, ec);
        if ( basic_result.empty() )
        {
            std::cerr << "[ERROR] Could not resolve address: " << partAddress << ":" << partPort << "\n";
            std::terminate();
        }

        auto endpoint = basic_result.begin()->endpoint();
        return endpoint;
    }

    GameClient::GameClient()
        : m_state( std::monostate{} )
    {
    }

    int GameClient::RunService( const std::string & realmlist, const std::string & username, const std::string & password )
    {
        std::thread thread( [&]
        {
            m_context.run();
        } );

        boost::asio::signal_set signals( m_context, SIGINT, SIGTERM );
        signals.async_wait( [&]( auto, auto )
        {
            m_context.stop();
        } );

        auto logonEndpoint = ParseEndpoint( m_context, realmlist );
        m_state.emplace< LoginState >( realmlist, username, password );

        while ( !m_context.stopped() )
        {
            const bool result = std::visit( [&]( auto & state )
            {
                if constexpr ( !std::is_same_v< std::monostate &, decltype( state ) > )
                    return UpdateState( state );

                return true;
            }, m_state );

            if ( !result )
                break;
        }

        m_context.stop();
        thread.join();

        return EXIT_SUCCESS;
    }

    bool GameClient::UpdateState( LoginState & state )
    {
        if ( !state.m_session )
        {
            state.m_session.emplace( m_context );

            auto endpoint = ParseEndpoint( m_context, state.m_logonServer );
            if ( !state.m_session->Connect( endpoint, state.m_username, state.m_password ) )
                return false;
        }

        state.m_session->Update();

        if ( auto realmlist = state.m_session->GetRealmList() )
        {
            std::cout << "\nRealmlist: \n";
            for ( auto & info : *realmlist )
            {
                std::cout << " - " << info.name << " (" << info.address << ")\n";
            }

            if ( realmlist->empty() )
                return false;

            auto & realm = realmlist->front();
            auto cryptoKey = state.m_session->GetCredentials().K;

            m_state.emplace< GameState >( realm.address, cryptoKey );
            return true;
        }

        return state.m_session->IsConnected();
    }

    bool GameClient::UpdateState( GameState & state )
    {
        if ( !state.m_session )
        {
            state.m_session.emplace( m_context, state.m_cryptoKey );

            auto endpoint = ParseEndpoint( m_context, state.m_realmServer );
            if ( !state.m_session->Connect( endpoint ) )
                return false;
        }

        state.m_session->Update();

        return state.m_session->IsConnected();
    }
}
