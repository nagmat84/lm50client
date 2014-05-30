#ifndef _WORKER_RRD_H_
#define _WORKER_RRD_H_

#include "daemon_worker.h"

namespace LM50 {

class WorkerRrd : public DaemonWorker {
	public:
		WorkerRrd( ModeDaemon &parent );
		virtual ~WorkerRrd() {}
		
	public:
		virtual int run();
};

}

#endif