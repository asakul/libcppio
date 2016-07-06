
#ifndef IOLINEMANAGER_H
#define IOLINEMANAGER_H

#include "ioline.h"
#include "visibility.h"

#include <memory>

namespace cppio
{

class CPPIO_API IoLineManager
{
public:
	IoLineManager();
	virtual ~IoLineManager();

	IoLine* createClient(const std::string& address);
	IoAcceptor* createServer(const std::string& address);

	void registerFactory(std::unique_ptr<IoLineFactory> factory);

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
};

CPPIO_API std::shared_ptr<IoLineManager> createLineManager();

}

#endif /* ifndef IOLINEMANAGER_H */
