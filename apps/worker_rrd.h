#ifndef _WORKER_RRD_H_
#define _WORKER_RRD_H_

#include "daemon_worker.h"
#include "program_options.h"

namespace LM50 {

class WorkerRrd : public DaemonWorker {
	public:
		WorkerRrd( ModeDaemon &parent );
		virtual ~WorkerRrd();
		
	public:
		virtual int run();
		
	private:
		void obtainValues();
		void stepBeat();
		void sleepUntilBeat() const;
		void updateRRD();
		void logHeader() const;
		void logValues() const;
		
	private:
		unsigned long _pollingPeriod;
		struct timespec _timeBeat;
		LM50Device::ChIdx _chSize;
		const ProgramOptions::ChList& _chIdx;
		LM50Device::ChVal* _chValues;
		struct timespec _timeUpdate;
};

}

#endif