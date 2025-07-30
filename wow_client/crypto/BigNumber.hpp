#pragma once

#include <vector>
#include <array>
#include <gsl/span>

struct bignum_st;

namespace Crypto
{
    class BigNumber
    {
    public:
        BigNumber();
        BigNumber( uint32_t );
        BigNumber( gsl::span< const std::byte > data );

        BigNumber( BigNumber const & bn );

        ~BigNumber();

        operator uint32_t() const;

        BigNumber & operator=( BigNumber const & bn );

        BigNumber& operator+=( BigNumber const & bn );
        BigNumber operator+( BigNumber const & bn ) const;

        BigNumber& operator-=( BigNumber const & bn );
        BigNumber operator-( BigNumber const & bn ) const;

        BigNumber& operator*=( BigNumber const & bn );
        BigNumber operator*( BigNumber const & bn ) const;

        BigNumber& operator/=( BigNumber const & bn );
        BigNumber operator/( BigNumber const & bn ) const;

        BigNumber& operator%=( BigNumber const & bn );
        BigNumber operator%( BigNumber const & bn ) const;

        template< size_t N >
        std::array< std::byte, N > GetFixedBytes() const
        {
            std::array< std::byte, N > result;

            auto bytes = GetBytes( N );
            for ( int idx = 0u; idx < N; ++idx )
            {
                result[ idx ] = bytes[ idx ];
            }

            return result;
        }

        std::vector<std::byte> GetBytes( size_t minSize = 0 ) const;

        BigNumber   ModExp( BigNumber const & bn1, BigNumber const & bn2 ) const;
        BigNumber   Exp( BigNumber const & );
        void        Randomize( uint8_t bytes );

        bool        IsZero() const;

    private:
        bignum_st * m_impl;
    };
}