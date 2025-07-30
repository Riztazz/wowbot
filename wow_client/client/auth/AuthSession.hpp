#pragma once

#include "crypto/Sha1.hpp"
#include "client/packets/Packets.hpp"
#include "networking/Socket.hpp"

#include <string>
#include <variant>
#include <deque>
#include <optional>

namespace Wow
{
    constexpr uint8_t CLIENT_VERSION_MAJOR = 3;
    constexpr uint8_t CLIENT_VERSION_MINOR = 3;
    constexpr uint8_t CLIENT_VERSION_PATCH = 5;
    constexpr uint16_t CLIENT_BUILD_NUMBER = 12340;

    enum class AuthStatus : uint8_t
    {
        Success = 0x00,
        Failure = 0x01,
        Unknown1 = 0x02,
        AccountBanned = 0x03,
        AccountNotFound = 0x04,
        Unknown2 = 0x05,
        AccountInUse = 0x06,
        PrepaidInvalid = 0x07,
        ServerIsFull = 0x08,
        InvalidClientBuild = 0x09,
        InvalidClientPatch = 0x0A,
        Unknown3 = 0x0b,
        AccountIsFrozen = 0x0C,
        Unknown4 = 0x0d,
        Unknown5 = 0x0e,
        ParrentalControl = 0x0f
    };

    using SessionPacket = std::variant< Auth::ServerLogonChallenge, Auth::ServerLogonProof, Auth::ServerRealmList >;

    struct Credentials
    {
        std::string m_username;
        std::string m_password;

        Crypto::BigNumber    K;
        Crypto::Sha1::Digest M2;
    };

    class AuthSession : public Network::Socket< Auth::ServerOpcode >
    {
    public:
        AuthSession( boost::asio::io_context & context );

        bool                            Connect( Endpoint endpoint, std::string_view username, std::string_view password );

        void                            Update();

        auto                            GetRealmList() const { return m_realmList; }
        const Credentials &             GetCredentials() const { return m_credentials; }

    private:
        void                            HandlePacket( const Auth::ServerLogonChallenge & packet );
        void                            HandlePacket( const Auth::ServerLogonProof & packet );
        void                            HandlePacket( const Auth::ServerRealmList & packet );

        void                            SendLogonChallenge();
        void                            SendLogonProof( const Crypto::BigNumber & A, const Crypto::Sha1::Digest & M1 );
        void                            SendRealmListQuery();

        boost::asio::awaitable<bool>    ReceivePacketAsync( Auth::ServerOpcode opcode ) override;

        boost::asio::awaitable<bool>    ReceiveServerLogonChallengeAsync();
        boost::asio::awaitable<bool>    ReceiveServerLogonProofAsync();
        boost::asio::awaitable<bool>    ReceiveServerRealmListAsync();

        std::optional< std::vector< Auth::ServerRealmInfo > > m_realmList;

        std::mutex                      m_mutex;
        std::deque<SessionPacket>       m_queue;

        Credentials                     m_credentials;
    };
}
