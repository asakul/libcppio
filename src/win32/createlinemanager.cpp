
#include "cppio/iolinemanager.h"

#include "../common/inproc.h"
#include "pipes.h"
#include "win_socket.h"

namespace cppio
{
	CPPIO_API IoLineManager* createLineManager()
	{
		auto manager = new IoLineManager();
		manager->registerFactory(std::unique_ptr<InprocLineFactory>(new InprocLineFactory));
		manager->registerFactory(std::unique_ptr<NamedPipeLineFactory>(new NamedPipeLineFactory));
		manager->registerFactory(std::unique_ptr<WinSocketFactory>(new WinSocketFactory));
		return manager;
	}
}

