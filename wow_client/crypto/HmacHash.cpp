//#include "HmacHash.hpp"

namespace Crypto
{
    HmacHash::HmacHash( gsl::span< const std::byte > seed )
    {
        //HMAC_CTX_init( &m_context );
        //HMAC_Init_ex( &m_context,  (const uint8_t* )seed.data(), seed.size(), EVP_sha1(), nullptr );
    }

    HmacHash::~HmacHash()
    {
        //HMAC_CTX_cleanup( &m_context );
    }

    HmacHash::Digest HmacHash::ComputeHash( gsl::span< const std::byte > data )
    {
        //HMAC_Update( &m_context, ( const uint8_t* )data.data(), data.size() );
        //
        //Digest result = {};
        //
        //uint32_t length = 0u;
        //HMAC_Final( &m_context, ( uint8_t* )result.data(), &length );
        //
        //return result;
        Digest result{};
        return result;
    }
}
