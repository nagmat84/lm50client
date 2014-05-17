#ifndef _MB_TCPCOMM_H_
#define _MB_TCPCOMM_H_

#include "mb_tcprar.h"

#include <string>
#include <sys/types.h>
#include <boost/asio.hpp>


namespace ModBus {

class TcpCommunication {
	public:
		/**
		 * Standard c'tor
		 */
		TcpCommunication();
		
		/**
		 * Creates a new object and calls
		 * TcpCommunication::open( const std::string&, const std::string& )
		 */
		TcpCommunication( const std::string& remoteHost, const std::string& port = std::string( "502" ) );
		
		virtual ~TcpCommunication() {
			close();
		}
		
	public:
		/**
		 * Opens the socket (using IPv4) and connects to the remote host
		 * @param remotHost The DNS name of the remote host
		 * @param port The port on that the remote host listens (default is "502").
		 * The port can either be a numerical value or well-known string that is
		 * an alias for the port (e.g. "http" means "80"; under Linux this aliases
		 * can be found in /etc/services)
		 */
		void open( const std::string& remoteHost, const std::string& port = std::string( "502" ) );
		
		/**
		 * Disconnects from the remote host and closes the socket
		 */
		void close();
		
		/**
		 * Creates and returns an object of TcpRequestAndReply that is binded
		 * to the socket managed by this object
		 * @param request The ModBus request 
		 */
		TcpRequestAndReply createRequestAndReply( const Datagram::Base& request );
		
		/**
		 * Creates and returns an object of TcpRequestAndReply that is binded
		 * to the socket managed by this object
		 * @param request The ModBus request 
		 * @param timeout The timeout for request
		 */
		TcpRequestAndReply createRequestAndReply( const Datagram::Base& request, const boost::posix_time::time_duration& timeout );
		
		const boost::asio::ip::tcp::socket& socket() const { return tcpSocket; }
		
	protected:
		boost::asio::io_service ioService;
		boost::asio::ip::tcp::socket tcpSocket;
};

}

#endif
