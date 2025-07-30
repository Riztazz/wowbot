#pragma once

#include "networking/ByteBuffer.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/thread/concurrent_queues/sync_deque.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/co_spawn.hpp>

namespace Network
{
    template< typename PacketHeader, size_t BUFFER_SIZE = 2048 >
    class Socket
    {
    public:
        using Endpoint = boost::asio::ip::tcp::endpoint;

        Socket( boost::asio::io_context & context )
            : m_buffer{}
            , m_context( context )
            , m_socket( context )
        {
        }

        virtual ~Socket()
        {
            if ( m_socket.is_open() )
                m_socket.close();
        }

        bool IsConnected() const
        {
            return m_socket.is_open();
        }

        boost::system::error_code Connect( Endpoint endpoint )
        {
            boost::system::error_code error;
            m_socket.connect( endpoint, error );

            if ( error )
                return error;

            boost::asio::co_spawn( m_context, [this]
            {
                return SocketReaderAsync();
            }, boost::asio::detached );

            return error;
        }

        uint32_t GetLocalAddress() const
        {
            if ( !IsConnected() )
                return -1;

            return m_socket.local_endpoint().address().to_v4().to_uint();
        }

        uint32_t GetRemoteAddress() const
        {
            if ( !IsConnected() )
                return -1;

            return m_socket.remote_endpoint().address().to_v4().to_uint();
        }

        void SendBuffer( ByteBuffer & buffer )
        {
            //! TODO: make it async
            boost::system::error_code error;
            boost::asio::write( m_socket, boost::asio::buffer( buffer.m_data.data(), buffer.m_data.size() ), error );

            if ( error )
                std::terminate();
        }

    protected:
        virtual boost::asio::awaitable<bool> ReceivePacketAsync( PacketHeader header ) = 0;

        template< typename ...T >
        auto ReadAsync() -> std::enable_if_t< sizeof...( T ) >= 2u, boost::asio::awaitable< std::tuple<T...> > >
        {
            static_assert( sizeof( std::tuple<T...> ) <= BUFFER_SIZE );

            const size_t bytesReceived = co_await boost::asio::async_read( m_socket, boost::asio::buffer( m_buffer.data(), sizeof( std::tuple<T...> ) ), boost::asio::use_awaitable );
            if ( bytesReceived != sizeof( std::tuple<T...> ) )
                std::terminate();

            co_return *reinterpret_cast< std::tuple<T... > * >( m_buffer.data() );
        }

        template< typename T >
        auto ReadAsync() -> boost::asio::awaitable< T >
        {
            static_assert( std::is_pod_v<T> );
            static_assert( sizeof( T ) <= BUFFER_SIZE );

            const size_t bytesReceived = co_await boost::asio::async_read( m_socket, boost::asio::buffer( m_buffer.data(), sizeof( T ) ), boost::asio::use_awaitable );
            if ( bytesReceived != sizeof( T ) )
                std::terminate();

            co_return *reinterpret_cast< T * >( m_buffer.data() );
        }

        auto ReadAsync( size_t bytesCount ) -> boost::asio::awaitable< ByteBuffer >
        {
            const size_t bytesReceived = co_await boost::asio::async_read( m_socket, boost::asio::buffer( m_buffer.data(), bytesCount ), boost::asio::use_awaitable );
            if ( bytesReceived != bytesCount )
                std::terminate();

            ByteBuffer buffer;
            buffer.m_data.resize( bytesCount );

            std::memcpy( buffer.m_data.data(), m_buffer.data(), bytesCount );
            co_return buffer;
        }

    private:
        boost::asio::awaitable<void> SocketReaderAsync()
        {
            while ( m_socket.is_open() )
            {
                auto header = co_await ReadAsync<PacketHeader>();

                const bool handled = co_await ReceivePacketAsync( std::move( header ) );
                if ( !handled )
                    std::terminate();
            }
        }

        std::array< std::byte, BUFFER_SIZE >        m_buffer;
        boost::concurrent::sync_deque< ByteBuffer > m_outgoing;

        boost::asio::io_context &                   m_context;
        boost::asio::ip::tcp::socket                m_socket;
    };
}
