
#include "catch.hpp"

#include "cppio/iolinemanager.h"
#include "win32/win_socket.h"

#include <numeric>
#include <thread>
#ifdef __MINGW32__
#ifndef _GLIBCXX_HAS_GTHREADS
#include "mingw.thread.h"
#endif // _GLIBCXX_HAS_GTHREADS
#endif
#include <unistd.h>

using namespace cppio;

static void checkIo(const std::shared_ptr<IoLineManager>& manager, const std::string& endpoint)
{
	std::array<char, 1024> buf;
	std::iota(buf.begin(), buf.end(), 0);

	auto server = std::unique_ptr<IoAcceptor>(manager->createServer(endpoint));

	auto client = std::unique_ptr<IoLine>(manager->createClient(endpoint));
	REQUIRE(client);

	auto socket = std::unique_ptr<IoLine>(server->waitConnection(100));

	int rc = client->write(buf.data(), 1024);
	REQUIRE(rc == 1024);

	std::array<char, 1024> recv_buf;
	socket->read(recv_buf.data(), 1024);

	REQUIRE(buf == recv_buf);

	std::fill(recv_buf.begin(), recv_buf.end(), 0);

	socket->write(buf.data(), 1024);

	rc = client->read(recv_buf.data(), 1024);
	REQUIRE(rc == 1024);

	REQUIRE(buf == recv_buf);
}

static void threadedCheckIo(const std::shared_ptr<IoLineManager>& manager, const std::string& endpoint, const std::string& serverEndpoint)
{
	std::array<char, 1024> buf;
	std::iota(buf.begin(), buf.end(), 0);

	std::array<char, 1024> recv_buf;

	std::thread serverThread([&]() {
			auto server = std::unique_ptr<IoAcceptor>(manager->createServer(serverEndpoint));
			auto socket = std::unique_ptr<IoLine>(server->waitConnection(100));
			socket->read(recv_buf.data(), 1024);
			});


	std::thread clientThread([&]() {
			usleep(10000);
			auto client = std::unique_ptr<IoLine>(manager->createClient(endpoint));
			REQUIRE(client);

			int rc = client->write(buf.data(), 1024);
			REQUIRE(rc == 1024);
			});


	serverThread.join();
	clientThread.join();

	REQUIRE(buf == recv_buf);
}

TEST_CASE("Win32: TCP socket", "[io]")
{
	auto manager = std::make_shared<IoLineManager>();
	manager->registerFactory(std::unique_ptr<WinSocketFactory>(new WinSocketFactory));

	threadedCheckIo(manager, "tcp://127.0.0.1:6000", "tcp://*:6000");
}

