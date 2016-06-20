
#ifndef IOLINEMANAGER_H
#define IOLINEMANAGER_H

#include "cppio/ioline.h"

#include <memory>

namespace cppio
{

class IoLineManager
{
public:
	IoLineManager();
	virtual ~IoLineManager();

	std::shared_ptr<IoLine> createClient(const std::string& address);
	std::shared_ptr<IoAcceptor> createServer(const std::string& address);

	void registerFactory(std::unique_ptr<IoLineFactory> factory);

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
};

}

#endif /* ifndef IOLINEMANAGER_H */
