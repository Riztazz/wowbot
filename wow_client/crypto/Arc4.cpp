#include "Arc4.hpp"

namespace Crypto
{
    Arc4::Arc4( gsl::span< const std::byte> seed )
    {
        //EVP_CIPHER_CTX_init( &m_context );
        //EVP_EncryptInit_ex( &m_context, EVP_rc4(), nullptr, nullptr, nullptr );
        //EVP_CIPHER_CTX_set_key_length( &m_context, seed.size() );
        //EVP_EncryptInit_ex( &m_context, nullptr, nullptr, ( const uint8_t* )seed.data(), nullptr );
    }

    Arc4::~Arc4()
    {
        //EVP_CIPHER_CTX_cleanup( &m_context );
    }

    void Arc4::Process( gsl::span<std::byte> data )
    {
        int size = 0;

        //EVP_EncryptUpdate( &m_context, ( uint8_t* )data.data(), &size, ( const uint8_t* )data.data(), data.size() );
        //EVP_EncryptFinal_ex( &m_context, ( uint8_t* )data.data(), &size );
    }
}
