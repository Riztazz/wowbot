#pragma once

#include "auth/AuthSession.hpp"
#include "game/GameSession.hpp"

#include <boost/asio/io_context.hpp>

#include <variant>
#include <optional>

namespace Wow
{
    struct LoginState
    {
        LoginState( const std::string & realmlist, const std::string & username, const std::string & password )
            : m_username( username )
            , m_password( password )
            , m_logonServer( realmlist )
        {
        }

        std::string m_logonServer;
        std::string m_username;
        std::string m_password;

        std::optional< AuthSession > m_session;
    };

    struct GameState
    {
        GameState( const std::string & realmlist, const Crypto::BigNumber & key )
            : m_cryptoKey( key )
            , m_realmServer( realmlist )
        {
        }

        std::string                     m_realmServer;
        const Crypto::BigNumber         m_cryptoKey;
        std::optional< GameSession >    m_session;
    };

    class GameClient
    {
    public:
        GameClient();

        int                         RunService( const std::string & realmlist, const std::string & username, const std::string & password );

    private:
        bool                        UpdateState( LoginState & state );
        bool                        UpdateState( GameState & session );

        boost::asio::io_context                                 m_context;
        std::variant< std::monostate, LoginState, GameState >   m_state;
    };
}
