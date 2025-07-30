#include "BigNumber.hpp"

#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <xutility>

namespace Crypto
{
    BigNumber::BigNumber()
        : m_impl( BN_new() )
    {
    }

    BigNumber::BigNumber( BigNumber const & bn )
        : m_impl( BN_dup( bn.m_impl ) )
    {
    }

    BigNumber::BigNumber( uint32_t val )
        : m_impl( BN_new() )
    {
        BN_set_word( m_impl, val );
    }

    BigNumber::BigNumber( gsl::span< const std::byte > data )
        : m_impl( BN_new() )
    {
        std::vector< std::byte > buffer;
        buffer.insert( buffer.end(), data.rbegin(), data.rend() );

        BN_bin2bn( reinterpret_cast< const uint8_t* >( buffer.data() ), ( int )buffer.size(), m_impl );
    }

    BigNumber::~BigNumber()
    {
        BN_free( m_impl );
    }

    BigNumber & BigNumber::operator=( BigNumber const & bn )
    {
        if ( this == &bn )
            return *this;

        if ( m_impl == nullptr )
        {
            m_impl = BN_dup( bn.m_impl );
        }
        else
        {
            BN_copy( m_impl, bn.m_impl );
        }
        return *this;
    }

    BigNumber::operator uint32_t() const
    {
        return ( uint32_t ) BN_get_word( m_impl );
    }

    void BigNumber::Randomize( uint8_t bytes )
    {
        BN_rand( m_impl, bytes * 8, 0, 1 );
    }

    BigNumber& BigNumber::operator+=( BigNumber const & bn )
    {
        BN_add( m_impl, m_impl, bn.m_impl );
        return *this;
    }

    BigNumber BigNumber::operator+( BigNumber const & bn ) const
    {
        BigNumber t( *this );
        return t += bn;
    }

    BigNumber& BigNumber::operator-=( BigNumber const & bn )
    {
        BN_sub( m_impl, m_impl, bn.m_impl );
        return *this;
    }

    BigNumber BigNumber::operator-( BigNumber const & bn ) const
    {
        BigNumber t( *this );
        return t -= bn;
    }

    BigNumber& BigNumber::operator*=( BigNumber const & bn )
    {
        BN_CTX * bnctx = BN_CTX_new();
        BN_mul( m_impl, m_impl, bn.m_impl, bnctx );
        BN_CTX_free( bnctx );

        return *this;
    }

    BigNumber BigNumber::operator*( BigNumber const & bn ) const
    {
        BigNumber t( *this );
        return t *= bn;
    }

    BigNumber& BigNumber::operator/=( BigNumber const & bn )
    {
        BN_CTX * bnctx = BN_CTX_new();
        BN_div( m_impl, NULL, m_impl, bn.m_impl, bnctx );
        BN_CTX_free( bnctx );

        return *this;
    }

    BigNumber BigNumber::operator/( BigNumber const & bn ) const
    {
        BigNumber t( *this );
        return t /= bn;
    }

    BigNumber& BigNumber::operator%=( BigNumber const & bn )
    {
        BN_CTX * bnctx = BN_CTX_new();
        BN_mod( m_impl, m_impl, bn.m_impl, bnctx );
        BN_CTX_free( bnctx );

        return *this;
    }

    BigNumber BigNumber::operator%( BigNumber const & bn ) const
    {
        BigNumber t( *this );
        return t %= bn;
    }

    std::vector<std::byte> BigNumber::GetBytes( size_t minSize ) const
    {
        size_t numBytes = BN_num_bytes( m_impl );
        size_t size = ( minSize >= numBytes ) ? minSize : numBytes;

        std::vector<std::byte> bytes;
        bytes.resize( size, std::byte{} );

        BN_bn2bin( m_impl, reinterpret_cast< uint8_t * >( bytes.data() ) );
        std::reverse( bytes.begin(), bytes.end() );

        return bytes;
    }

    BigNumber BigNumber::Exp( BigNumber const & bn )
    {
        BigNumber ret;
        BN_CTX * bnctx;

        bnctx = BN_CTX_new();
        BN_exp( ret.m_impl, m_impl, bn.m_impl, bnctx );
        BN_CTX_free( bnctx );

        return ret;
    }

    bool BigNumber::IsZero() const
    {
        return BN_is_zero( m_impl );
    }

    BigNumber BigNumber::ModExp( BigNumber const & bn1, BigNumber const & bn2 ) const
    {
        BigNumber ret;

        BN_CTX * bnctx = BN_CTX_new();
        BN_mod_exp( ret.m_impl, m_impl, bn1.m_impl, bn2.m_impl, bnctx );
        BN_CTX_free( bnctx );

        return ret;
    }
}
