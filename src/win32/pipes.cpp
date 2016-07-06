
#include "pipes.h"

#ifdef _GLIBCXX_HAVE_BROKEN_VSWPRINTF
#include <sstream>
template < typename T > std::string to_string( const T& n )
{
    std::ostringstream stm ;
    stm << n ;
    return stm.str() ;
}
#else
using std::to_string;
#endif // _GLIBCXX_HAVE_BROKEN_VSWPRINTF

namespace cppio
{

	static const std::string pipePrefix = "\\\\.\\pipe\\";

NamedPipeLine::NamedPipeLine(const std::string& address)
{
	HANDLE serverPipe = CreateFile((pipePrefix + address).c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, 0, NULL);
	if(serverPipe == INVALID_HANDLE_VALUE) // FIXME handle ERROR_PIPE_BUSY
		throw IoException("[1]Unable to open pipe: " + to_string(GetLastError()));

	DWORD dwMode = PIPE_READMODE_MESSAGE;
	if(!SetNamedPipeHandleState(serverPipe, &dwMode, NULL, NULL))
	{
		int error = GetLastError();
		if(error)
			throw IoException("[2]Unable to open pipe: " + to_string(GetLastError()));
	}

	std::array<char, 1024> pipeNameBuffer;

	DWORD cbRead;
	ReadFile(serverPipe, pipeNameBuffer.data(), pipeNameBuffer.size(), &cbRead, NULL);
	CloseHandle(serverPipe);

	m_address = std::string(pipeNameBuffer.data(), cbRead);
	if(m_address.empty())
		throw IoException("Invalid new address");
	m_pipe = CreateFile((pipePrefix + m_address).c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, 0, NULL);
	if(m_pipe == INVALID_HANDLE_VALUE) // FIXME handle ERROR_PIPE_BUSY
		throw IoException("[3]Unable to open pipe: " + to_string(GetLastError()));
}

NamedPipeLine::NamedPipeLine(HANDLE fd, const std::string& address) : m_address(address),
	m_pipe(fd)
{
}

NamedPipeLine::~NamedPipeLine()
{
}

ssize_t NamedPipeLine::read(void* buffer, size_t buflen)
{
	DWORD rd;
	if(!ReadFile(m_pipe, buffer, buflen, &rd, NULL))
		return 0;
	return rd;
}

ssize_t NamedPipeLine::write(void* buffer, size_t buflen)
{
	DWORD wr;
	if(!WriteFile(m_pipe, buffer, buflen, &wr, NULL))
		return 0;
	return wr;
}

void NamedPipeLine::setOption(LineOption option, void* data)
{
}

NamedPipeAcceptor::NamedPipeAcceptor(const std::string& address) : m_address(address)
{
	m_pipe = CreateNamedPipe((pipePrefix + address).c_str(), PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			8192, 8192, 0, NULL);
	if(m_pipe == INVALID_HANDLE_VALUE)
		throw IoException("Unable to create pipe: " + to_string(GetLastError()));
}

NamedPipeAcceptor::~NamedPipeAcceptor()
{
	CloseHandle(m_pipe);
}

static bool connectPipe(HANDLE pipe, int msec)
{
	OVERLAPPED ol = {0,0,0,0,NULL};
	BOOL ret = 0;

	ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ret = ConnectNamedPipe(pipe, &ol);
    if(ret == 0)
	{
		switch(GetLastError())
		{
			case ERROR_PIPE_CONNECTED:
				ret = TRUE;
				break;
			case ERROR_IO_PENDING:
				if(WaitForSingleObject(ol.hEvent, msec) == WAIT_OBJECT_0)
				{
					DWORD dwIgnore;
					ret = GetOverlappedResult(pipe, &ol, &dwIgnore, FALSE);
				}
				else
				{
					CancelIo(pipe);
				}
				break;
		}
	}
	CloseHandle(ol.hEvent);
	return ret != 0;
}

IoLine* NamedPipeAcceptor::waitConnection(int timeoutInMs)
{
	std::string newPipeAddress = m_address + to_string(m_counter.fetch_add(1));
	m_waitingPipe = CreateNamedPipe((pipePrefix + newPipeAddress).c_str(), PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_BYTE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			65536, 65536, 0, NULL);

	bool ret = connectPipe(m_pipe, timeoutInMs);
	if(!ret)
	{
		CloseHandle(m_waitingPipe);
		return nullptr;
	}

	DWORD written = 0;
	WriteFile(m_pipe, newPipeAddress.c_str(), newPipeAddress.size(), &written, NULL);
	// TODO handle written != newPipeAddress.size();
	FlushFileBuffers(m_pipe);
	DisconnectNamedPipe(m_pipe);

	ret = connectPipe(m_waitingPipe, timeoutInMs);
	if(!ret)
	{
		CloseHandle(m_waitingPipe);
		return nullptr;
	}
	return new NamedPipeLine(m_waitingPipe, newPipeAddress);
}

NamedPipeLineFactory::NamedPipeLineFactory()
{
}

NamedPipeLineFactory::~NamedPipeLineFactory()
{
}

bool NamedPipeLineFactory::supportsScheme(const std::string& scheme)
{
	return scheme == "local";
}

IoLine* NamedPipeLineFactory::createClient(const std::string& address)
{
	return new NamedPipeLine(address);
}

IoAcceptor* NamedPipeLineFactory::createServer(const std::string& address)
{
	return new NamedPipeAcceptor(address);
}

}


