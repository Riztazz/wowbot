#pragma once

#include "crypto/Sha1.hpp"
#include <array>
#include "networking/ByteBuffer.hpp"

#pragma pack(push, 1)

namespace Wow
{
    template< size_t N >
    using FixedArray = std::array< std::byte, N >;

    namespace Auth
    {
        enum class ClientOpcode : uint8_t
        {
            LogonChallenge  = 0x00,
            LogonProof      = 0x01,
            RealmList       = 0x10
        };

        enum class ServerOpcode : uint8_t
        {
            LogonChallenge  = 0x00,
            LogonProof      = 0x01,
            RealmList       = 0x10
        };

        struct ServerLogonChallenge
        {
            FixedArray< 32 > B;
            uint8_t          GLength;
            uint8_t          G;
            uint8_t          NSize;
            FixedArray< 32 > N;
            FixedArray< 32 > S;
            FixedArray< 16 > Unk3;
            uint8_t          SecurityFlags;
        };

        struct ServerLogonProof
        {
            Crypto::Sha1::Digest M2;
            uint32_t             unk1;
            uint32_t             unk2;
            uint16_t             unk3;
        };

        enum class RealmFlags : uint8_t
        {
            None             = 0x00,
            VersionMismatch  = 0x01,
            Offline          = 0x02,
            SpecifyBuild     = 0x04,
            Unk1             = 0x08,
            Unk2             = 0x10,
            Recommended      = 0x20,
            New              = 0x40,
            Full             = 0x80
        };

        struct ServerRealmInfo
        {
            uint8_t             icon;
            uint8_t             lock;
            RealmFlags          flags;
            Network::NullString name;
            Network::NullString address;
            float               populationLevel;
            uint8_t             charactersCount;
            uint8_t             timezone;
            uint8_t             realmId;
        };

        using ServerRealmList = std::vector<ServerRealmInfo>;
    }

    namespace Game
    {
        enum class ClientOpcode : uint32_t
        {
            AuthSession         = 0x1ED,
        };

        enum class ServerOpcode : uint16_t
        {
            AuthChallenge       = 0x1EC,
        };

        struct ServerAuthChallenge
        {
            uint32_t         SeedSize;
            uint32_t         Seed;

            FixedArray< 16 > Seed1;
            FixedArray< 16 > Seed2;
        };
    }
};

#pragma pack(pop)
