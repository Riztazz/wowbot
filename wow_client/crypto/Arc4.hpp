#pragma once

#include <openssl/evp.h>
#include <gsl/span>

namespace Crypto
{
    class Arc4
    {
    public:
        Arc4( gsl::span<const std::byte> data );
        ~Arc4();

        void            Process( gsl::span<std::byte> data );

    private:
        //EVP_CIPHER_CTX  m_context;
    };
}
