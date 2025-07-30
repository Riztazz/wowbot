#pragma once

#include "networking/Socket.hpp"
#include "client/packets/Packets.hpp"
#include "crypto/Arc4.hpp"

#include <deque>
#include <gsl/span>
#include <optional>

namespace Wow
{
    struct PacketCrypto
    {
        const std::array<std::uint8_t, 16> ENCRYPTION_KEY =
        {
            0xC2, 0xB3, 0x72, 0x3C, 0xC6, 0xAE, 0xD9, 0xB5,
            0x34, 0x3C, 0x53, 0xEE, 0x2F, 0x43, 0x67, 0xCE
        };

        const std::array<std::uint8_t, 16> DECRYPTION_KEY =
        {
            0xCC, 0x98, 0xAE, 0x04, 0xE8, 0x97, 0xEA, 0xCA,
            0x12, 0xDD, 0xC0, 0x93, 0x42, 0x91, 0x53, 0x57
        };

        PacketCrypto() = default;

        void                            Initialize( const Crypto::BigNumber & seed );

        void                            Decrypt( gsl::span< std::byte > buffer );
        void                            Encrypt( gsl::span< std::byte > buffer );

        std::optional<Crypto::Arc4>     m_decrypt;
        std::optional<Crypto::Arc4>     m_encrypt;
    };

    class GameSession : public Network::Socket<std::byte>
    {
    public:
        GameSession( boost::asio::io_context & context, const Crypto::BigNumber & key );

        bool    Connect( Endpoint endpoint );
        void    Update();

    private:
        void                            HandleAuthChallenge( Network::ByteBuffer packet );

        boost::asio::awaitable<bool>    ReceivePacketAsync( std::byte header ) override;

        using PacketQueue = std::deque< std::pair< Game::ServerOpcode, Network::ByteBuffer > >;

        std::mutex                      m_mutex;
        PacketQueue                     m_queue;
        std::optional<PacketCrypto>     m_crypto;
        const Crypto::BigNumber         m_cryptoKey;
    };
}
