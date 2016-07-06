
#include "visibility.h"
#include <stddef.h>
#include <sys/types.h>

CPPIO_API enum cppio_line_option
{
	cppio_receive_timeout = 1,
	cppio_send_timeout = 2
};

extern "C"
{
	typedef void* cppio_iolinemanager;
	typedef void* cppio_ioline;
	typedef void* cppio_ioacceptor;
	typedef void* cppio_message;
	typedef void* cppio_messageprotocol;

	CPPIO_API cppio_iolinemanager cppio_create_line_manager();

	CPPIO_API cppio_ioline cppio_create_client(cppio_iolinemanager manager, const char* address);
	CPPIO_API void cppio_destroy_line(cppio_ioline line);

	CPPIO_API cppio_ioacceptor cppio_create_server(cppio_iolinemanager manager, const char* address);
	CPPIO_API void cppio_destroy_acceptor(cppio_ioacceptor acceptor);

	cppio_ioline cppio_acceptor_wait_connection(cppio_ioacceptor acceptor, int timeout);
	ssize_t cppio_line_read(cppio_ioline line, char* buffer, size_t buffer_length);
	ssize_t cppio_line_write(cppio_ioline line, char* buffer, size_t buffer_length);
	int cppio_line_set_option(cppio_ioline line, cppio_line_option option, void* data);

	cppio_message cppio_create_message();
	void cppio_message_add(cppio_message message, const char* buffer, size_t buffer_length);
	size_t cppio_message_size(cppio_message message);
	const char* cppio_message_get_frame(cppio_message message, size_t part_number);
	size_t cppio_message_get_frame_length(cppio_message message, size_t part_number);
	void cppio_message_clear(cppio_message message);
	void cppio_destroy_message(cppio_message message);

	cppio_messageprotocol cppio_create_messageprotocol(cppio_ioline line);
	void cppio_destroy_messageprotocol(cppio_messageprotocol protocol);
	size_t cppio_messageprotocol_send(cppio_messageprotocol protocol, cppio_message message);
	size_t cppio_messageprotocol_read(cppio_messageprotocol protocol, cppio_message message);
}

