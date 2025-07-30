#pragma once

#include "crypto/BigNumber.hpp"

#include <openssl/sha.h>
#include <array>
#include <string>

namespace Crypto
{
    class Sha1
    {
    public:
        Sha1();
        ~Sha1();

        using Digest = std::array<std::byte, SHA_DIGEST_LENGTH>;

        template< typename ...T >
        static Digest CalculateHash( T && ... data )
        {
            Sha1 sha1;

            sha1.UpdateMultiData( std::forward< T >( data )... );
            return sha1.Finalize();
        }

    private:
        template< typename ...T >
        void UpdateMultiData( T && ... data )
        {
            ( UpdateData( data ), ... );
        }

        void            UpdateData( const BigNumber & bn );
        void            UpdateData( gsl::span< const std::byte > data );
        void            UpdateData( std::string_view data );

        void            Initialize();
        const Digest &  Finalize();

        SHA_CTX                                  m_context;
        std::array<std::byte, SHA_DIGEST_LENGTH> m_digest;
    };
}
