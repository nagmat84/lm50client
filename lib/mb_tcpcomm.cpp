#include "lib/mb_tcpcomm.h"

namespace ModBus {

TcpCommunication::TcpCommunication() : ioService(), tcpSocket( ioService ) {

}


TcpCommunication::TcpCommunication( const std::string& remoteHost, const std::string& port ) : ioService(), tcpSocket( ioService )  {
	open( remoteHost, port );
}


void TcpCommunication::open( const std::string& remoteHost, const std::string& port ) {
	close();
	
	boost::asio::ip::tcp::resolver resolver( ioService );
	boost::asio::ip::tcp::resolver::query query( boost::asio::ip::tcp::v4(), remoteHost, port );
	boost::asio::ip::tcp::endpoint remote_endpoint = *resolver.resolve( query );
	tcpSocket.open( boost::asio::ip::tcp::v4() );
	tcpSocket.connect( remote_endpoint );
}


void TcpCommunication::close() {
	tcpSocket.close();
}


TcpRequestAndReply TcpCommunication::createRequestAndReply( const ModBus::Datagram::Base& request ) {
	return ModBus::TcpRequestAndReply( tcpSocket, request );
}


TcpRequestAndReply TcpCommunication::createRequestAndReply( const ModBus::Datagram::Base& request, const boost::posix_time::time_duration& timeout ) {
	return ModBus::TcpRequestAndReply( tcpSocket, request, timeout );
}



}
