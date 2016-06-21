
#include "cppio/iolinemanager.h"

#include "../common/inproc.h"
#include "pipes.h"
#include "win_socket.h"

namespace cppio
{
	std::shared_ptr<IoLineManager> createLineManager()
	{
		auto manager = std::make_shared<IoLineManager>();
		manager->registerFactory(std::unique_ptr<InprocLineFactory>(new InprocLineFactory));
		manager->registerFactory(std::unique_ptr<NamedPipeLineFactory>(new NamedPipeLineFactory));
		manager->registerFactory(std::unique_ptr<WinSocketFactory>(new WinSocketFactory));
		return manager;
	}
}

