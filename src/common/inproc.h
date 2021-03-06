
#ifndef INPROC_H
#define INPROC_H

#include "cppio/ioline.h"

#include <cstddef>

#ifndef CPPIO_BLOCKING_INPROC
#include <atomic>
#endif

#include <vector>
#include <mutex>
#ifdef __MINGW32__
#ifndef _GLIBCXX_HAS_GTHREADS
#include "mingw.mutex.h"
#include "mingw.condition_variable.h"
#endif
#endif
#include <condition_variable>

namespace cppio
{

class RingBuffer
{
public:
	RingBuffer(size_t bufferSize);
	~RingBuffer();

	size_t read(void* buffer, size_t buflen);
	size_t write(void* buffer, size_t buflen);

	size_t readPointer() const { return m_rdptr; }
	size_t writePointer() const { return m_wrptr; }

	size_t availableReadSize() const;
	size_t availableWriteSize() const;

	size_t size() const { return m_data.size(); }
private:
	std::vector<char> m_data;
#ifdef CPPIO_BLOCKING_INPROC
	size_t m_wrptr;
	size_t m_rdptr;
#else
	std::atomic<size_t> m_wrptr;
	std::atomic<size_t> m_rdptr;
#endif
};

class DataQueue
{
public:
	DataQueue(size_t bufferSize);
	~DataQueue();

	ssize_t read(void* buffer, size_t buflen);
	ssize_t write(void* buffer, size_t buflen);

	ssize_t readWithTimeout(void* buffer, size_t buflen, const std::chrono::milliseconds& timeout);

	size_t readPointer() const { return m_buffer.readPointer(); }
	size_t writePointer() const { return m_buffer.writePointer(); }

	size_t availableReadSize() const;
	size_t availableWriteSize() const;

	void setConnectionFlag(bool c);
private:
	RingBuffer m_buffer;

	std::mutex m_mutex;
	std::condition_variable m_readCondition;
	std::condition_variable m_writeCondition;
	bool m_connected;
};

class InprocLine : public IoLine
{
public:
	InprocLine(InprocLine* other);
	InprocLine(const std::string& address);
	virtual ~InprocLine();

	virtual ssize_t read(void* buffer, size_t buflen) override;
	virtual ssize_t write(void* buffer, size_t buflen) override;
	virtual void setOption(LineOption option, void* data);

	std::string address() const { return m_address; }

	void waitForConnection();

private:
	std::string m_address;
	std::mutex m_mutex;
	std::condition_variable m_condition;

	std::shared_ptr<DataQueue> m_in;
	std::shared_ptr<DataQueue> m_out;

	int m_readTimeout;
};

class InprocAcceptor : public IoAcceptor
{
public:
	InprocAcceptor(const std::string& address);
	virtual ~InprocAcceptor();

	virtual IoLine* waitConnection(int timeoutInMs) override;

	std::string address() const { return m_address; }


private:
	std::string m_address;
};

class InprocLineFactory : public IoLineFactory
{
public:
	InprocLineFactory();
	virtual ~InprocLineFactory();

	virtual bool supportsScheme(const std::string& scheme) override;
	virtual IoLine* createClient(const std::string& address) override;
	virtual IoAcceptor* createServer(const std::string& address) override;
};

}

#endif /* ifndef INPROC_H */
