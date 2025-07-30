#include "Sha1.hpp"

namespace Crypto
{
    Sha1::Sha1()
    {
        SHA1_Init( &m_context );
        std::memset( m_digest.data(), 0, m_digest.size() );
    }

    Sha1::~Sha1()
    {
        SHA1_Init( &m_context );
    }

    void Sha1::UpdateData( const BigNumber & bn )
    {
        UpdateData( bn.GetBytes() );
    }

    void Sha1::UpdateData( std::string_view data )
    {
        SHA1_Update( &m_context, data.data(), data.size() );
    }

    void Sha1::UpdateData( gsl::span< const std::byte > data )
    {
        SHA1_Update( &m_context, data.data(), data.size() );
    }

    void Sha1::Initialize()
    {
        SHA1_Init( &m_context );
    }

    const Sha1::Digest & Sha1::Finalize()
    {
        SHA1_Final( reinterpret_cast< uint8_t* >( m_digest.data() ), &m_context );
        SHA1_Init( &m_context );

        return m_digest;
    }
}
