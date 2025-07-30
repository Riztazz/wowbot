#include "AuthSession.hpp"

#include "crypto/BigNumber.hpp"
#include "crypto/Sha1.hpp"
#include "client/packets/Packets.hpp"

#include <boost/asio/ip/address.hpp>
#include <boost/asio/co_spawn.hpp>
#include <optional>
#include <iostream>
#include <gsl/span>
#include <mutex>

namespace Wow
{
    AuthSession::AuthSession( boost::asio::io_context & context )
        : Socket( context )
        , m_credentials{}
    {
    }

    boost::asio::awaitable<bool> AuthSession::ReceivePacketAsync( Auth::ServerOpcode opcode )
    {
        //! maybe `co_return co_await` is not needed? Can we transform and forward received awaitable?
        switch ( opcode )
        {
            case Auth::ServerOpcode::LogonChallenge: co_return co_await ReceiveServerLogonChallengeAsync();
            case Auth::ServerOpcode::LogonProof:     co_return co_await ReceiveServerLogonProofAsync();
            case Auth::ServerOpcode::RealmList:      co_return co_await ReceiveServerRealmListAsync();
        }

        co_return false;
    }

    bool AuthSession::Connect( Endpoint endpoint, std::string_view username, std::string_view password )
    {
        const auto error = Socket::Connect( endpoint );
        if ( error )
        {
            std::cerr << "[ERROR] " << error << "\n";
            return false;
        }

        m_credentials.m_username = username;
        m_credentials.m_password = password;

        SendLogonChallenge();
        return true;
    }

    void AuthSession::Update()
    {
        std::lock_guard lock( m_mutex );
        while ( !m_queue.empty() )
        {
            std::visit( [this]( auto & p )
            {
                HandlePacket( p );
            }, m_queue.front() );
            m_queue.pop_front();
        }
    }

    void AuthSession::SendLogonChallenge()
    {
        std::cout << "[INFO] SendLogonChallenge\n";

        Network::ByteBuffer packet;
        packet << Auth::ClientOpcode::LogonChallenge;
        packet << ( uint8_t )6;
        packet << ( uint16_t )( m_credentials.m_username.size() + 30 );
        packet << ( uint32_t )'WoW';
        packet << CLIENT_VERSION_MAJOR;
        packet << CLIENT_VERSION_MINOR;
        packet << CLIENT_VERSION_PATCH;
        packet << CLIENT_BUILD_NUMBER;
        packet << ( uint32_t )'68x';
        packet << ( uint32_t )'niW';
        packet << ( uint32_t )'SUne';
        packet << ( uint32_t )0x3c;
        packet << GetLocalAddress();
        packet << Network::SizeStringView( m_credentials.m_username );

        SendBuffer( packet );
    }

    void AuthSession::SendLogonProof( const Crypto::BigNumber & A, const Crypto::Sha1::Digest & M1 )
    {
        std::cout << "[INFO] SendLogonProof\n";

        Network::ByteBuffer packet;
        packet << Auth::ClientOpcode::LogonProof;
        packet << A.GetFixedBytes< 32 >();
        packet << M1;
        packet << std::array<std::byte, 20>();
        packet << ( uint8_t )0;
        packet << ( uint8_t )0;

        SendBuffer( packet );
    }

    void AuthSession::SendRealmListQuery()
    {
        std::cout << "[INFO] SendRealmListQuery\n";

        Network::ByteBuffer packet;
        packet << Auth::ClientOpcode::RealmList;
        packet << ( uint8_t )0;
        packet << ( uint8_t )0;
        packet << ( uint8_t )0;
        packet << ( uint8_t )0;

        SendBuffer( packet );
    }

    void AuthSession::HandlePacket( const Auth::ServerLogonChallenge & packet )
    {
        const auto B = Crypto::BigNumber( packet.B );
        const auto g = Crypto::BigNumber( gsl::span< const std::byte >( ( std::byte * ) & packet.G, 1 ) );
        const auto N = Crypto::BigNumber( packet.N );
        const auto Salt = Crypto::BigNumber( packet.S );
        const auto Unk3 = Crypto::BigNumber( packet.Unk3 );

        //! SRP6 implementation
        const auto k = Crypto::BigNumber( 3 ); // multiplier

        const auto I = Crypto::Sha1::CalculateHash( m_credentials.m_username, ":", m_credentials.m_password );
        const auto x = Crypto::BigNumber( Crypto::Sha1::CalculateHash( Salt, I ) ); // private key
        const auto v = g.ModExp( x, N );                                            // password verifier

        Crypto::BigNumber a;
        a.Randomize( 19 );

        Crypto::BigNumber A = g.ModExp( a, N );

        auto uHash = Crypto::Sha1::CalculateHash( A, B );
        const auto u = Crypto::BigNumber( uHash );

        Crypto::BigNumber S = ( B - k * v ).ModExp( ( a + u * x ), N );

        auto t = S.GetBytes( 32 );

        std::array<std::byte, 16> t1{};
        for ( size_t idx = 0; idx < t1.size(); ++idx )
        {
            t1[ idx ] = t[ idx * 2 ];
        }

        std::array<std::byte, 40> vK{};

        auto t1Hash = Crypto::Sha1::CalculateHash( t1 );
        for ( size_t idx = 0; idx < t1Hash.size(); ++idx )
        {
            vK[ idx * 2 ] = t1Hash[ idx ];
        }

        for ( size_t idx = 0; idx < t1.size(); ++idx )
        {
            t1[ idx ] = t[ idx * 2 + 1 ];
        }

        t1Hash = Crypto::Sha1::CalculateHash( t1 );
        for ( size_t idx = 0; idx < t1Hash.size(); ++idx )
        {
            vK[ idx * 2 + 1 ] = t1Hash[ idx ];
        }

        const auto K = Crypto::BigNumber( vK );

        auto nHash = Crypto::Sha1::CalculateHash( N );
        auto gHash = Crypto::Sha1::CalculateHash( g );

        for ( size_t idx = 0; idx < gHash.size(); ++idx )
        {
            nHash[ idx ] ^= gHash[ idx ];
        }

        const auto t3 = Crypto::BigNumber( nHash );

        const auto M = Crypto::Sha1::CalculateHash( m_credentials.m_username );
        const auto M1 = Crypto::Sha1::CalculateHash( t3, M, Salt, A, B, K );

        m_credentials.K = K;
        m_credentials.M2 = Crypto::Sha1::CalculateHash( A, M1, K );

        SendLogonProof( A, M1 );
    }

    void AuthSession::HandlePacket( const Auth::ServerLogonProof & packet )
    {
        const bool isProofValid = std::equal( packet.M2.begin(), packet.M2.end(), m_credentials.M2.begin(), m_credentials.M2.end() );
        if ( !isProofValid )
            return SendLogonChallenge();

        SendRealmListQuery();
    }

    void AuthSession::HandlePacket( const Auth::ServerRealmList & packet )
    {
        if ( packet.empty() )
            return SendRealmListQuery();

        m_realmList = packet;
    }

    boost::asio::awaitable<bool> AuthSession::ReceiveServerLogonChallengeAsync()
    {
        auto [_, status] = co_await ReadAsync< uint8_t, AuthStatus >();
        if ( status != AuthStatus::Success )
            co_return true;

        auto challenge = co_await ReadAsync< Auth::ServerLogonChallenge >();

        std::lock_guard lock( m_mutex );
        m_queue.push_back( challenge );

        co_return true;
    }

    boost::asio::awaitable<bool> AuthSession::ReceiveServerLogonProofAsync()
    {
        auto status = co_await ReadAsync< AuthStatus >();
        if ( status != AuthStatus::Success )
        {
            co_await ReadAsync< uint16_t >();
            co_return false;
        }

        auto proof = co_await ReadAsync< Auth::ServerLogonProof >();

        std::lock_guard lock( m_mutex );
        m_queue.push_back( proof );

        co_return true;
    }

    boost::asio::awaitable<bool> AuthSession::ReceiveServerRealmListAsync()
    {
        auto packetSize = co_await ReadAsync< uint16_t >();

        Network::ByteBuffer packet = co_await ReadAsync( packetSize );

        uint32_t unk1 = 0u;
        unk1 << packet;

        uint16_t realmsCount = 0u;
        realmsCount << packet;

        Auth::ServerRealmList realmlist;
        realmlist.reserve( realmsCount );

        for ( auto idx = 0u; idx < realmsCount; ++idx )
        {
            Auth::ServerRealmInfo & info = realmlist.emplace_back();
            info.icon << packet;
            info.lock << packet;
            info.flags << packet;
            info.name << packet;
            info.address << packet;
            info.populationLevel << packet;
            info.charactersCount << packet;
            info.timezone << packet;
            info.realmId << packet;

            if ( ( int )info.flags & ( int )Auth::RealmFlags::SpecifyBuild )
            {
                uint8_t majorVersion = 0u;
                majorVersion << packet;

                uint8_t minorVersion = 0u;
                minorVersion << packet;

                uint8_t patchVersion = 0u;
                patchVersion << packet;

                uint16_t buildNumber = 0u;
                buildNumber << packet;
            }
        }

        uint8_t unk2 = 0u;
        unk2 << packet;

        uint8_t unk3 = 0u;
        unk3 << packet;

        std::lock_guard lock( m_mutex );
        m_queue.emplace_back( std::move( realmlist ) );

        co_return true;
    }
}
