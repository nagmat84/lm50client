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
		enum ReportPeriod { DAILY, WEEKLY, MONTHLY };
	
	public:
		ProgramOptions();
		virtual ~ProgramOptions() {}
		
	public:
		void parse( int argCount, char* argVals[] );
		
		void print( std::ostream &s ) const;
		
		OperationMode operationMode() const { return _operationMode; }
		
		bool hasOptionHelp() const { return _hasOptionHelp; }
		
		const std::string& host() const { return _host; }
		
		const std::string& port() const { return _port; }
		
		bool stayInForeground() const { return _foreground; }
		
		bool beVerbose() const { return _verbose; }
		
		const ChList& channels() const { return _channels; }
		
		bool rrdEnabled() const { return _rrd; }
		
		const std::string& rrdFile() const { return _rrdFile; }
		
		unsigned long rrdPeriod() const { return _rrdPeriod; }
		
		bool reportEnabled() const { return _report; }
		
		const std::string& reportFile() const { return _reportFile; }
		
		ReportPeriod reportPeriod() const { return _reportPeriod; }
		
		const std::string& reportRecipient() const { return _reportRecipient; }
		
	protected:
		void operationMode( OperationMode m );
		
	private:
		boost::program_options::options_description _commonOptions;
		boost::program_options::options_description _commonOptionsRRD;
		boost::program_options::options_description _commonOptionsReport;
		boost::program_options::options_description _cmdLineOnlyOptions;
		boost::program_options::variables_map _optionVMap;
		OperationMode _operationMode;
		bool _hasOptionHelp;
		std::string _configFile;
		std::string _host;
		std::string _port; // port is a string, because telling name (i.e. 'http' insted of 80) are valid, too
		bool _foreground;
		bool _verbose;
		ChList _channels;
		bool _rrd;
		std::string _rrdFile;
		unsigned long _rrdPeriod; // polling period in seconds
		bool _report;
		std::string _reportFile;
		ReportPeriod _reportPeriod;
		std::string _reportRecipient;
};

}

#endif
