
#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include "cppio/iolinemanager.h"
#include "cppio/ioline.h"
#include "cppio/message.h"
#include "cppio/cppio_c.h"

using namespace boost::python;
using namespace cppio;


class GIL
{
public:
	GIL()
	{
		gstate = PyGILState_Ensure();
	}

	~GIL()
	{
		PyGILState_Release(gstate);
	}

private:
	PyGILState_STATE gstate;
};

class ScopedGILRelease
{
public:
    inline ScopedGILRelease()
	{
		m_thread_state = PyEval_SaveThread();
	}
    inline ~ScopedGILRelease()
	{
        PyEval_RestoreThread(m_thread_state);
        m_thread_state = NULL;
    }
private:
    PyThreadState* m_thread_state;
};

IoLineManager* makeIoLineManager()
{
	return cppio::createLineManager();
}

handle<> getMessageFrame(const Message& msg, int frameNumber)
{
	auto f = msg.frame(frameNumber);
	return handle<>(PyByteArray_FromStringAndSize(static_cast<const char*>(f.data()), f.size()));
}

void appendMessageFrame(Message& msg, handle<>& bytes)
{
	if(PyByteArray_Check(bytes.get()))
		msg.addFrame(Frame(PyByteArray_AsString(bytes.get()), PyByteArray_Size(bytes.get())));
	else if(PyBytes_Check(bytes.get()))
		msg.addFrame(Frame(PyBytes_AsString(bytes.get()), PyBytes_Size(bytes.get())));
	else
		throw std::runtime_error("Invalid object passed, should be of type 'bytes' or 'bytearray'");
}

static handle<> readFromLine(IoLine* line, int toread)
{
	size_t done = 0;
	std::vector<char> buffer(toread);
	{
		ScopedGILRelease r;
		done = line->read(buffer.data(), toread);
	}
	return handle<>(PyByteArray_FromStringAndSize(static_cast<const char*>(buffer.data()), done));
}

static void writeToLine(IoLine* line, handle<>& data)
{
	if(PyByteArray_Check(data.get()))
	{
		char* begin = PyByteArray_AsString(data.get());
		char* end = begin + PyByteArray_Size(data.get());
		std::vector<char> buffer(begin, end);

		{
			ScopedGILRelease r;
			line->write(buffer.data(), buffer.size());
		}
	}
	else if(PyBytes_Check(data.get()))
	{
		char* begin = PyBytes_AsString(data.get());
		char* end = begin + PyBytes_Size(data.get());
		std::vector<char> buffer(begin, end);

		{
			ScopedGILRelease r;
			line->write(buffer.data(), buffer.size());
		}
	}
	else
		throw std::runtime_error("Invalid object passed, should be of type 'bytes' or 'bytearray'");
}

IoLine* waitConnection(IoAcceptor* self, float seconds)
{
	ScopedGILRelease r;
	return self->waitConnection(seconds * 1000);
}

IoLine* IoLineManager_createClient(IoLineManager* self, const std::string& address)
{
	ScopedGILRelease r;
	return self->createClient(address);
}

Message MessageProtocol_readMessage(MessageProtocol& self)
{
	ScopedGILRelease r;
	Message m;
	self.readMessage(m);
	return m;
}

void MessageProtocol_sendMessage(MessageProtocol& self, const Message& msg)
{
	ScopedGILRelease r;
	self.sendMessage(msg);
}

BOOST_PYTHON_MODULE(pycppio)
{
	// Create GIL
	if(!PyEval_ThreadsInitialized())
	{
		PyEval_InitThreads();
	}

	class_<IoLineManager, boost::noncopyable>("IoLineManager", no_init)
		.def("createClient", IoLineManager_createClient, return_value_policy<manage_new_object>())
		.def("createServer", &IoLineManager::createServer, return_value_policy<manage_new_object>())
		;

	def("makeIoLineManager", makeIoLineManager, return_value_policy<manage_new_object>());

	class_<Message>("Message")
		.def(init<>())
		.def("size", &Message::size)
		.def("getFrame", getMessageFrame)
		.def("appendFrame", appendMessageFrame)
		;

	class_<IoLine, boost::noncopyable>("IoLine", no_init)
		.def("read", readFromLine)
		.def("write", writeToLine)
		;

	class_<IoAcceptor, boost::noncopyable>("IoAcceptor", no_init)
		.def("waitConnection", waitConnection, return_value_policy<manage_new_object>())
	;

	class_<MessageProtocol, boost::noncopyable>("MessageProtocol", init<IoLine*>())
		.def("sendMessage", MessageProtocol_sendMessage)
		.def("readMessage", MessageProtocol_readMessage, return_value_policy<return_by_value>())
		;
}

