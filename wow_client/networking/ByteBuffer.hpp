#pragma once

#include <vector>
#include <string_view>
#include <gsl/span>

namespace Network
{
    struct NullStringView : public std::string_view
    {
        NullStringView( std::string_view str = {} )
            : std::string_view( str )
        {
        }
    };

    template< typename Type = uint8_t >
    struct BaseSizeStringView : public std::string_view
    {
        BaseSizeStringView( std::string_view str = {} )
            : std::string_view( str )
        {
        }
    };

    using SizeStringView = BaseSizeStringView< uint8_t >;

    struct NullString : public std::string
    {
        NullString( std::string_view str = {} )
            : std::string( str )
        {
        }
    };

    template< typename Type = uint8_t >
    struct BaseSizeString : public std::string
    {
        BaseSizeString( std::string_view str = {} )
            : std::string( str )
        {
        }
    };

    using SizeString = BaseSizeString< uint8_t >;

    struct ByteBuffer
    {
        template< typename T >
        ByteBuffer & operator<<( T data ) noexcept
        {
            static_assert( std::is_pod_v< T > && !std::is_pointer_v<T>, "ERROR: only POD data supported!" );

            auto index = m_data.size();
            m_data.resize( index + sizeof( T ) );

            std::memcpy( &m_data[ index ], &data, sizeof( T ) );
            return *this;
        }

        template<>
        ByteBuffer & operator<<( NullStringView str ) noexcept
        {
            auto index = m_data.size();
            m_data.resize( index + str.size() + 1 );
            std::memcpy( &m_data[ index ], str.data(), str.size() );
            m_data.back() = std::byte{ 0 };

            return *this;
        }

        template<typename U>
        ByteBuffer & operator<<( BaseSizeStringView< U > str ) noexcept
        {
            *this << static_cast< U >( str.size() );

            size_t index = m_data.size();
            m_data.resize( index + str.size() );

            std::memcpy( &m_data[ index ], str.data(), str.size() );
            return *this;
        }

        template< typename T >
        T * Convert()
        {
            static_assert( std::is_pod_v< T > && !std::is_pointer_v<T>, "ERROR: only POD data supported!" );

            if ( sizeof( T ) != m_data.size() )
                std::terminate();

            return reinterpret_cast< T * >( m_data.data() );
        }

        size_t                   m_readOffset = 0u;
        std::vector< std::byte > m_data;
    };
}

template< typename T >
inline Network::ByteBuffer & operator<<( T & dst, Network::ByteBuffer & buffer )
{
    static_assert( std::is_pod_v< T > && !std::is_pointer_v<T>, "ERROR: only POD data supported!" );

    if ( ( sizeof( T ) + buffer.m_readOffset ) > buffer.m_data.size() )
        std::terminate();

    std::memcpy( &dst, &( buffer.m_data[ buffer.m_readOffset ] ), sizeof( T ) );
    buffer.m_readOffset += sizeof( T );
    return buffer;
}

inline Network::ByteBuffer & operator<<( Network::NullString & dst, Network::ByteBuffer & buffer )
{
    auto begin = std::next( buffer.m_data.begin(), buffer.m_readOffset );
    auto end = std::find( begin, buffer.m_data.end(), std::byte{} );

    size_t size = std::distance( begin, end );

    dst = std::string_view( reinterpret_cast< char * >( &( buffer.m_data[ buffer.m_readOffset ] ) ), size );
    buffer.m_readOffset += size + 1;

    return buffer;
}

template< typename U >
inline Network::ByteBuffer & operator<<( Network::BaseSizeString< U > & dst, Network::ByteBuffer & buffer )
{
    U stringSize = {};
    stringSize << buffer;

    if ( ( stringSize + buffer.m_readOffset ) > buffer.m_data.size() )
        std::terminate();

    dst = std::string_view( ( char * )&buffer.m_data[ buffer.m_readOffset ], stringSize );
    buffer.m_readOffset += stringSize;

    return buffer;
}
