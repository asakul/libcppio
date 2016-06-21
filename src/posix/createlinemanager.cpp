
#include "cppio/iolinemanager.h"

#include "../common/inproc.h"
#include "io_socket.h"

namespace cppio
{
	CPPIO_API std::shared_ptr<IoLineManager> createLineManager()
	{
		auto manager = std::make_shared<IoLineManager>();
		manager->registerFactory(std::unique_ptr<InprocLineFactory>(new InprocLineFactory));
		manager->registerFactory(std::unique_ptr<UnixSocketFactory>(new UnixSocketFactory));
		manager->registerFactory(std::unique_ptr<TcpSocketFactory>(new TcpSocketFactory));
		return manager;
	}
}

