
#ifndef WIN32_SOCKET_H
#define WIN32_SOCKET_H

#include "cppio/ioline.h"

#include <windows.h>

namespace cppio
{

class WinSocket : public IoLine
{
public:
	WinSocket(const std::string& address);
	WinSocket(SOCKET fd, const std::string& address);
	virtual ~WinSocket();

	virtual void connect();

	virtual ssize_t read(void* buffer, size_t buflen);
	virtual ssize_t write(void* buffer, size_t buflen);
	virtual void setOption(LineOption option, void* data);

private:
	std::string m_address;
	SOCKET m_socket;
};

class WinSocketAcceptor : public IoAcceptor
{
public:
	WinSocketAcceptor(const std::string& address);
	virtual ~WinSocketAcceptor();

	virtual IoLine* waitConnection(int timeoutInMs);

private:
	std::string m_address;
	SOCKET m_socket;
};

class WinSocketFactory : public IoLineFactory
{
public:
	WinSocketFactory();
	virtual ~WinSocketFactory();
	virtual bool supportsScheme(const std::string& scheme);
	virtual IoLine* createClient(const std::string& address);
	virtual IoAcceptor* createServer(const std::string& address);
};

}

#endif /* ifndef WIN32_SOCKET_H */
