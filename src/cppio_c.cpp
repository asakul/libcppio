
#include "cppio/cppio_c.h"
#include "cppio/iolinemanager.h"
#include "cppio/ioline.h"
#include "cppio/message.h"

using namespace cppio;
extern "C"
{
	CPPIO_API cppio_iolinemanager cppio_create_line_manager()
	{
		return createLineManager();
	}

	CPPIO_API cppio_ioline cppio_create_client(cppio_iolinemanager manager, const char* address)
	{
		try
		{
			auto man = static_cast<IoLineManager*>(manager);
			return man->createClient(std::string(address));
		}
		catch(const IoException& e)
		{
			return nullptr;
		}
	}

	CPPIO_API void cppio_destroy_line(cppio_ioline line)
	{
		delete static_cast<IoLine*>(line);
	}

	CPPIO_API cppio_ioacceptor cppio_create_server(cppio_iolinemanager manager, const char* address)
	{
		try
		{
			auto man = static_cast<IoLineManager*>(manager);
			return man->createServer(std::string(address));
		}
		catch(const IoException& e)
		{
			return nullptr;
		}
	}

	CPPIO_API void cppio_destroy_acceptor(cppio_ioacceptor acceptor)
	{
		delete static_cast<IoAcceptor*>(acceptor);
	}

	cppio_ioline cppio_acceptor_wait_connection(cppio_ioacceptor acceptor, int timeout)
	{
		auto a = static_cast<IoAcceptor*>(acceptor);
		return a->waitConnection(timeout);
	}

	ssize_t cppio_line_read(cppio_ioline line, char* buffer, size_t buffer_length)
	{
		auto l = static_cast<IoLine*>(line);
		return l->read(buffer, buffer_length);
	}

	ssize_t cppio_line_write(cppio_ioline line, char* buffer, size_t buffer_length)
	{
		auto l = static_cast<IoLine*>(line);
		return l->write(buffer, buffer_length);
	}

	int cppio_line_set_option(cppio_ioline line, cppio_line_option option, void* data)
	{
		auto l = static_cast<IoLine*>(line);
		l->setOption((LineOption)option, data);
		return 0;
	}

	cppio_message cppio_create_message()
	{
		return new Message();
	}

	void cppio_message_add(cppio_message message, const char* buffer, size_t buffer_length)
	{
		auto m = static_cast<Message*>(message);
		m->addFrame(Frame(buffer, buffer_length));
	}

	size_t cppio_message_size(cppio_message message)
	{
		auto m = static_cast<Message*>(message);
		return m->size();
	}

	const char* cppio_message_get_frame(cppio_message message, size_t part_number)
	{
		auto m = static_cast<Message*>(message);
		return static_cast<const char*>(m->frame(part_number).data());
	}

	size_t cppio_message_get_frame_length(cppio_message message, size_t part_number)
	{
		auto m = static_cast<Message*>(message);
		return m->frame(part_number).size();
	}

	void cppio_message_clear(cppio_message message)
	{
		auto m = static_cast<Message*>(message);
		m->clear();
	}

	void cppio_destroy_message(cppio_message message)
	{
		auto m = static_cast<Message*>(message);
		delete m;
	}

	cppio_messageprotocol cppio_create_messageprotocol(cppio_ioline line)
	{
		return new MessageProtocol(static_cast<IoLine*>(line));
	}

	void cppio_destroy_messageprotocol(cppio_messageprotocol protocol)
	{
		delete static_cast<MessageProtocol*>(protocol);
	}

	size_t cppio_messageprotocol_send(cppio_messageprotocol protocol, cppio_message message)
	{
		auto proto = static_cast<MessageProtocol*>(protocol);
		return proto->sendMessage(*static_cast<Message*>(message));
	}

	size_t cppio_messageprotocol_read(cppio_messageprotocol protocol, cppio_message message)
	{
		auto proto = static_cast<MessageProtocol*>(protocol);
		return proto->readMessage(*static_cast<Message*>(message));
	}
}

