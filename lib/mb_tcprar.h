#ifndef _mb_tcprar_h_
#define _mb_tcprar_h_


#include "mb_base.h"
#include <boost/asio.hpp>


namespace ModBus {

/**
 * An object of this class manages a typical request-and-reply round in
 * a ModBus/TCP communication. It uses a TCP socket to transmit the given
 * request and listens for the corresponding reply.
 */
class TcpRequestAndReply {
	public:
		/**
		 * Standard c'tor
		 */
		TcpRequestAndReply();
		
		/**
		 * @param tcpSocket This socket is used to transmit and receive the
		 * datagrams. The socket is not needed to be already connected to a remote
		 * end point. But the socket must be connected before
		 * TcpRequestAndReply::run() is invoked.
		 * @param request The request to be sent
		 * @param duration The time to wait for a response. Default is 1 second.
		 */
		TcpRequestAndReply( boost::asio::ip::tcp::socket& tcpSocket, const Datagram::Base& request, const boost::asio::deadline_timer::duration_type& duration = boost::posix_time::seconds( 1 ) );
		
		TcpRequestAndReply( const TcpRequestAndReply& other );
		
		virtual ~TcpRequestAndReply();
	
	public:
		//void connect( const std::string& host );
		/**
		 * @param tcpSocket This socket is used to transmit and receive the
		 * datagrams. The socket is not needed to be already connected to a remote
		 * end point. But the socket must be connected before
		 * TcpRequestAndReply::run() is invoked.
		 */
		void bindTo( boost::asio::ip::tcp::socket& tcpSocket );
		
		/**
		 * @param request The request to be sent
		 */
		void request( const Datagram::Base& req );
		
		/**
		 * @param duration The time to wait for a response.  Default is 1 second.
		 */
		void timeOut( const boost::asio::deadline_timer::duration_type& d );
		
		/**
		 * Tries to transmit the given request with the given socket. The socket
		 * must be connected. If the socket does not exist, is not connected or
		 * if the request is not valid, an appropriate exception is thrown.
		 * The function blocks until either the response has arrived or the time
		 * out duration has expired.
		 */
		void run();
		
		/**
		 * @return The response for the request. If TcpRequestAndReply::run()
		 * has never been invoked or was unsuccessful, NULL is returned
		 */
		const Datagram::Base* response() const {
			return responseDgram;
		}
		
	protected:
		void handleDeadline( const boost::system::error_code& error );
		void handleReceive( const boost::system::error_code& error, std::size_t nBytes );
		void handleSend( const boost::system::error_code& error, std::size_t nBytes );
		
	protected:
		boost::asio::ip::tcp::socket* socket;
		boost::asio::deadline_timer* timer;
		boost::asio::deadline_timer::duration_type timeoutDuration;
		Datagram::Base* requestDgram;
		Datagram::Base* responseDgram;
		
		
		char responseBuffer[ ModBus::Datagram::Base::maxDatagramLength ];
		size_t nBytesSent;
		size_t nBytesReceived;
};

}

#endif
