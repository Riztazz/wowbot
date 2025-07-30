#pragma once

//#include <openssl/hmac.h>
//#include <openssl/sha.h>
#include <gsl/span>

namespace Crypto
{
    class HmacHash
    {
    public:
        HmacHash( gsl::span< const std::byte > seed );
        ~HmacHash();

        using Digest = std::array<std::byte, SHA_DIGEST_LENGTH>;

        Digest      ComputeHash( gsl::span< const std::byte > data );

    private:
        //HMAC_CTX    m_context;
    };
}
