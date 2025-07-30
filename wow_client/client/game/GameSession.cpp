#include "client/game/GameSession.hpp"
#include "crypto/HmacHash.hpp"
#include "networking/ByteBuffer.hpp"

#include <iostream>
#include <ios>

namespace Wow
{
    static Crypto::HmacHash::Digest GetDigest( gsl::span< const uint8_t > key, const Crypto::BigNumber & seed )
    {
        Crypto::HmacHash hmac( { ( const std::byte * ) key.data(), key.size() } );

        auto seedBytes = seed.GetBytes();
        return hmac.ComputeHash( seedBytes );
    }

    void PacketCrypto::Initialize( const Crypto::BigNumber & seed )
    {
        auto digest = GetDigest( DECRYPTION_KEY, seed );
        m_decrypt.emplace( digest );

        digest = GetDigest( DECRYPTION_KEY, seed );
        m_encrypt.emplace( digest );

        //! Drop first 1024 bytes
        std::array< std::byte, 1024 > dummy = {};

        m_decrypt->Process( dummy );
        m_encrypt->Process( dummy );
    }

    void PacketCrypto::Decrypt( gsl::span< std::byte > buffer )
    {
        if ( m_decrypt )
            m_decrypt->Process( buffer );
    }

    void PacketCrypto::Encrypt( gsl::span< std::byte > buffer )
    {
        if ( m_encrypt )
            m_encrypt->Process( buffer );
    }

    GameSession::GameSession( boost::asio::io_context & context, const Crypto::BigNumber & key )
        : Socket( context )
        , m_cryptoKey( key )
    {
    }

    bool GameSession::Connect( Endpoint endpoint )
    {
        const auto error = Socket::Connect( endpoint );
        if ( error )
        {
            std::cerr << "[ERROR] " << error << "\n";
            return false;
        }

        return true;
    }

    void GameSession::Update()
    {
        std::lock_guard lock( m_mutex );
        while( !m_queue.empty() )
        {
            auto [opcode, packet] = std::move( m_queue.front() );
            m_queue.pop_front();

            switch ( opcode )
            {
                case Game::ServerOpcode::AuthChallenge: HandleAuthChallenge( std::move( packet ) ); break;
                default:
                {
                    std::cerr << "ERROR: unhandled opcode: " << std::hex << ( uint32_t )opcode << "\n";
                    std::terminate();
                }
            }
        }
    }

    void GameSession::HandleAuthChallenge( Network::ByteBuffer packet )
    {
        Game::ServerAuthChallenge challenge;
        challenge << packet;


        std::cout << "[INFO] HandleAuthChallenge\n";

        Network::ByteBuffer response_packet;
        response_packet << Game::ClientOpcode::AuthSession;
        response_packet << ( uint8_t )0;
        response_packet << ( uint8_t )0;
        response_packet << ( uint8_t )0;
        response_packet << ( uint8_t )0;

        SendBuffer( packet );

    }

    boost::asio::awaitable<bool> GameSession::ReceivePacketAsync( std::byte header )
    {
        std::byte temp = header;
        m_crypto->Decrypt( { &temp, 1 } );

        const bool isLargePacket = ( ( uint8_t )header & 0x80 ) != 0;

        auto buffer = co_await ReadAsync( isLargePacket ? 4 : 3 );
        m_crypto->Decrypt( buffer.m_data );

        if ( !isLargePacket )
        {
            buffer.m_data.insert( buffer.m_data.begin(), header );
        }

        Game::ServerOpcode opcode = ( Game::ServerOpcode )( ( uint8_t )buffer.m_data[ 2 ] << 8 | ( uint8_t )buffer.m_data[ 3 ] );

        size_t packetSize = ( isLargePacket ? ( uint8_t )header & 0x7F << 16 : 0 ) | ( uint8_t )buffer.m_data[ 0 ] << 8 | ( uint8_t )buffer.m_data[ 1 ];

        auto packet = co_await ReadAsync( packetSize );

        {
            std::lock_guard lock( m_mutex );
            m_queue.emplace_back( opcode, std::move( packet ) );
        }

        co_return true;
    }
}
