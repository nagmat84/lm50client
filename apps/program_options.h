#ifndef _PROGRAM_OPTIONS_H_
#define _PROGRAM_OPTIONS_H_

#include <boost/program_options.hpp>
#include <string>
#include "core/lm50device.h"

namespace LM50 {

class ProgramOptions {
	public:
		typedef LM50Device::ChIdx ChIdx;
		typedef std::vector< ChIdx > ChList;
		enum OperationMode { UNKNOWN, HUMAN, CACTI, DAEMON };
	
	public:
		ProgramOptions();
		virtual ~ProgramOptions() {}
		
	public:
		void parse( int argCount, char* argVals[] );
		
		void print( std::ostream &s ) const { _boostOptDesc.print( s ); }
		
		OperationMode operationMode() const { return _operationMode; }
		
		bool hasOptionHelp() const { return _hasOptionHelp; }
		
		const std::string& host() const { return _host; }
		
		const std::string& port() const { return _port; }
		
		const ChList& channels() const { return _channels; }
		
	protected:
		void operationMode( OperationMode m );
		
	private:
		boost::program_options::options_description _boostOptDesc;
		boost::program_options::variables_map _boostVMap;
		OperationMode _operationMode;
		bool _hasOptionHelp;
		std::string _host;
		std::string _port; // port is a string, because telling name (i.e. 'http' insted of 80) are valid, too
		bool _foreground;
		boost::posix_time::seconds _pollingPeriod;
		ChList _channels;
};

}

#endif
