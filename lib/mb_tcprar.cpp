#include "lib/mb_tcprar.h"
#include "lib/nullptr.h"

#include <boost/bind.hpp>


namespace ModBus {

TcpRequestAndReply::TcpRequestAndReply() : socket( nullptr ), timer( nullptr ), timeoutDuration( boost::posix_time::seconds( 1 ) ), requestDgram( nullptr ), responseDgram( nullptr ), nBytesSent( 0 ), nBytesReceived( 0 ) {
	memset( responseBuffer, 0x00, ModBus::Datagram::Base::maxDatagramLength );
}


TcpRequestAndReply::TcpRequestAndReply( boost::asio::ip::tcp::socket& tcpSocket, const ModBus::Datagram::Base& request, const boost::posix_time::time_duration& duration ) : socket( &tcpSocket ), timer( nullptr ), timeoutDuration( duration ), requestDgram( request.copy() ), responseDgram( nullptr ), nBytesSent( 0 ), nBytesReceived( 0 ) {
	memset( responseBuffer, 0x00, ModBus::Datagram::Base::maxDatagramLength );
}


TcpRequestAndReply::TcpRequestAndReply( const TcpRequestAndReply& other ) : socket( other.socket ), timer( nullptr ), timeoutDuration( other.timeoutDuration ), requestDgram( nullptr ), responseDgram( nullptr ), nBytesSent( other.nBytesSent ), nBytesReceived( other.nBytesReceived ) {
	assert( other.timer == nullptr );
	if( other.requestDgram != nullptr ) requestDgram = other.requestDgram->copy();
	if( other.responseDgram != nullptr ) responseDgram = other.responseDgram->copy();
	memcpy( responseBuffer, other.responseBuffer, ModBus::Datagram::Base::maxDatagramLength );
}


TcpRequestAndReply::~TcpRequestAndReply() {
	if( requestDgram ) delete requestDgram;
	if( responseDgram ) delete responseDgram;
	assert( timer == nullptr );
}


void TcpRequestAndReply::bindTo( boost::asio::ip::tcp::socket& tcpSocket ) {
	socket = &tcpSocket;
}


void TcpRequestAndReply::request( const ModBus::Datagram::Base& req ) {
	if( requestDgram ) delete requestDgram;
	requestDgram = req.copy();
}


void TcpRequestAndReply::timeOut( const boost::posix_time::time_duration& d ) {
	timeoutDuration = d;
}


void TcpRequestAndReply::run() {
	if( socket == nullptr || !socket->is_open() ) throw std::logic_error( "socket is not connected" );
	if( !requestDgram ) throw std::logic_error( "no request given for transmission" );
	assert( timer == nullptr );
	
	// Reset the service object (in case there were former calls to run() )
	socket->get_io_service().reset();
	
	// Schedule transmission
	nBytesSent = 0 ;
	socket->async_send( boost::asio::buffer( requestDgram->rawDatagram(), requestDgram->totalLength() ), boost::bind( &TcpRequestAndReply::handleSend, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) );
	
	// Schedule receive
	memset( responseBuffer, 0x00, ModBus::Datagram::Base::maxDatagramLength );
	nBytesReceived = 0;
	socket->async_receive( boost::asio::buffer( responseBuffer, ModBus::Datagram::Base::maxDatagramLength ), boost::bind( &TcpRequestAndReply::handleReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) );
	
	// Schedule time out
	timer = new boost::asio::deadline_timer( socket->get_io_service() );
	timer->expires_from_now( timeoutDuration );
	timer->async_wait( boost::bind( &TcpRequestAndReply::handleDeadline, this, boost::asio::placeholders::error ) );
	
	try {
		// Do all three task simultaneously
		socket->get_io_service().run();
		
		// This point can be reached by two ways. Either the timeout occured
		// or the I/O operation was sucessfull. In the first case nBytesReceived
		// equals zero, otherwise not. If an error had happened, run() would have
		// thrown an exception, such that this point would never have been reached.
		delete timer;
		timer = nullptr;
		
		if( responseDgram ) {
			delete responseDgram;
			responseDgram = nullptr;
		}
		if( nBytesReceived != 0 ) responseDgram = ModBus::Datagram::Base::createObject( responseBuffer, nBytesReceived );
		
	} catch( ... ) {
		if( timer != nullptr ) {
			delete timer;
			timer = nullptr;
		}
		nBytesReceived = 0;
		nBytesSent = 0;
		memset( responseBuffer, 0x00, ModBus::Datagram::Base::maxDatagramLength );
		if( responseDgram != nullptr ) {
			delete responseDgram;
			responseDgram = nullptr;
		}
		throw;
	}
}


void TcpRequestAndReply::handleDeadline( const boost::system::error_code& error ) {
	if( !error ) {
		// Timer has expired, cancel outstanding I/O operations
		socket->cancel();
		return;
	}
	// If error equals boost::asio::error::operation_aborted, then the timer
	// has been stopped, because the I/O operations have finished successfully.
	// In that case the function just returns. In all other cases, the error
	// is thrown as an exception
	if( error != boost::asio::error::operation_aborted ) throw error;
}


void TcpRequestAndReply::handleReceive( const boost::system::error_code& error, size_t nBytes ) {
	if( !error ) {
		// No error
		nBytesReceived += nBytes;
		
		// First check if we need more bytes to capture.
		try {
			if( nBytesReceived < sizeof( ModBus::Datagram::Header ) || ModBus::Datagram::Base::missingBytes( responseBuffer, nBytesReceived ) > 0 ) {
				// If more bytes are needed, advance the timer into the future and
				// start a new receive
				timer->expires_from_now( timeoutDuration );
				socket->async_receive( boost::asio::buffer( responseBuffer + nBytesReceived , ModBus::Datagram::Base::maxDatagramLength - nBytesReceived ), boost::bind( &TcpRequestAndReply::handleReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) );
			} else {
				// If enough bytes have been received, cancel the timer
				timer->cancel();
			}
		} catch( ... ) {
			// If some exception occured, we forget everything we have ever received
			memset( responseBuffer, 0x00, ModBus::Datagram::Base::maxDatagramLength );
			nBytesReceived = 0;
			throw;
		}
	} else {
		// If some error occured, we forget everything we have ever received
		memset( responseBuffer, 0x00, ModBus::Datagram::Base::maxDatagramLength );
		nBytesReceived = 0;
	
		
		if( error != boost::asio::error::operation_aborted ) throw error;
	}
}


void TcpRequestAndReply::handleSend( const boost::system::error_code& error, size_t nBytes ) {
	if( !error ) {
		// No error
		nBytesSent += nBytes;
		if( nBytesSent < requestDgram->totalLength() ) {
			// If more bytes need to be transmitted, advance the timer into the future
			// and start a new transmit
			timer->expires_from_now( timeoutDuration );
			socket->async_send( boost::asio::buffer( requestDgram->rawDatagram() + nBytesSent, requestDgram->totalLength() - nBytesSent ), boost::bind( &TcpRequestAndReply::handleSend, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) );
		}
	} else {
		// If error equals boost::asio::error::operation_aborted, then the I/O
		// operations has been stopped, because the deadline timer has expired.
		// In that case the function just returns. In all other cases, the error is
		// thrown as an exception.	
		if( error != boost::asio::error::operation_aborted ) throw error;
	}
}


}
