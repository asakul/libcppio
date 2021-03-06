
#include "catch.hpp"

#include "common/inproc.h"
#include "cppio/iolinemanager.h"

#include <numeric>
#include <thread>
#ifdef __MINGW32__
#ifndef _GLIBCXX_HAS_GTHREADS
#include "mingw.thread.h"
#endif
#endif
#include <cstring>
#include <memory>

using namespace cppio;

TEST_CASE("InprocLine", "[io]")
{
	IoLineManager manager;
	manager.registerFactory(std::unique_ptr<InprocLineFactory>(new InprocLineFactory()));

	SECTION("Simple read/write")
	{
		std::array<char, 1024> buf;
		std::array<char, 1024> recv_buf {};
		std::iota(buf.begin(), buf.end(), 0);

		auto acceptor = std::unique_ptr<IoAcceptor>(manager.createServer("inproc://foo"));
		std::thread clientThread([&](){
				auto client = std::unique_ptr<IoLine>(manager.createClient("inproc://foo"));
				client->write(buf.data(), buf.size());
			});

		std::thread serverThread([&](){
				auto server = std::unique_ptr<IoLine>(acceptor->waitConnection(100));
				server->read(recv_buf.data(), recv_buf.size());
			});

		clientThread.join();
		serverThread.join();

		REQUIRE(buf == recv_buf);
	}

	SECTION("Big read/write")
	{
		std::vector<char> buf(100 * 1024 * 1024);
		std::vector<char> recv_buf(100 * 1024 * 1024);
		std::iota(buf.begin(), buf.end(), 0);

		const int chunkSize = 1024;
		int totalChunks = buf.size() / chunkSize;

		auto acceptor = std::unique_ptr<IoAcceptor>(manager.createServer("inproc://foo"));
		std::thread clientThread([&](){
				auto client = std::unique_ptr<IoLine>(manager.createClient("inproc://foo"));
				for(size_t i = 0; i < totalChunks; i++)
				{
					auto start = buf.data() + chunkSize * i;
					int towrite = chunkSize;
					while(towrite > 0)
					{
						size_t done = client->write(start, towrite);
						towrite -= done;
						start += done;
					}
				}
			});

		std::thread serverThread([&](){
				auto server = std::unique_ptr<IoLine>(acceptor->waitConnection(100));
				for(size_t i = 0; i < totalChunks; i++)
				{
					auto start = recv_buf.data() + chunkSize * i;
					int toread = chunkSize;
					while(toread > 0)
					{
						size_t done = server->read(start, toread);
						toread -= done;
						start += done;
					}
				}
			});

		clientThread.join();
		serverThread.join();

		REQUIRE(std::equal(buf.begin(), buf.end(), recv_buf.begin()));
	}

	SECTION("Big read/write with delays")
	{
		std::vector<char> buf(10 * 1024 * 1024);
		std::vector<char> recv_buf(10 * 1024 * 1024);
		std::iota(buf.begin(), buf.end(), 0);

		const int chunkSize = 1024;
		int totalChunks = buf.size() / chunkSize;

		auto acceptor = std::unique_ptr<IoAcceptor>(manager.createServer("inproc://foo"));
		std::thread clientThread([&](){
				auto client = std::unique_ptr<IoLine>(manager.createClient("inproc://foo"));
				for(size_t i = 0; i < totalChunks; i++)
				{
					if((rand() % 100) == 0)
						std::this_thread::sleep_for(std::chrono::milliseconds(20));
					auto start = buf.data() + chunkSize * i;
					int towrite = chunkSize;
					while(towrite > 0)
					{
						size_t done = client->write(start, towrite);
						towrite -= done;
						start += done;
					}
				}
			});

		std::thread serverThread([&](){
				auto server = std::unique_ptr<IoLine>(acceptor->waitConnection(100));
				for(size_t i = 0; i < totalChunks; i++)
				{
					if((rand() % 100) == 0)
						std::this_thread::sleep_for(std::chrono::milliseconds(20));
					auto start = recv_buf.data() + chunkSize * i;
					int toread = chunkSize;
					while(toread > 0)
					{
						size_t done = server->read(start, toread);
						toread -= done;
						start += done;
					}
				}
			});

		clientThread.join();
		serverThread.join();

		REQUIRE(std::equal(buf.begin(), buf.end(), recv_buf.begin()));
	}

	SECTION("Connection delayed at server side")
	{
		std::array<char, 1024> buf;
		std::array<char, 1024> recv_buf {};
		std::iota(buf.begin(), buf.end(), 0);

		auto acceptor = std::unique_ptr<IoAcceptor>(manager.createServer("inproc://foo"));
		std::thread clientThread([&](){
				auto client = std::unique_ptr<IoLine>(manager.createClient("inproc://foo"));
				client->write(buf.data(), buf.size());
			});

		std::thread serverThread([&](){
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				auto server = std::unique_ptr<IoLine>(acceptor->waitConnection(200));
				server->read(recv_buf.data(), recv_buf.size());
			});

		clientThread.join();
		serverThread.join();

		REQUIRE(buf == recv_buf);
	}

	SECTION("Connection delayed at client side")
	{
		std::array<char, 1024> buf;
		std::array<char, 1024> recv_buf {};
		std::iota(buf.begin(), buf.end(), 0);

		auto acceptor = std::unique_ptr<IoAcceptor>(manager.createServer("inproc://foo"));
		std::thread clientThread([&](){
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				auto client = std::unique_ptr<IoLine>(manager.createClient("inproc://foo"));
				client->write(buf.data(), buf.size());
			});

		std::thread serverThread([&](){
				auto server = std::unique_ptr<IoLine>(acceptor->waitConnection(200));
				server->read(recv_buf.data(), recv_buf.size());
			});

		clientThread.join();
		serverThread.join();

		REQUIRE(buf == recv_buf);
	}

	SECTION("Connection loss")
	{
		std::array<char, 1024> buf;
		std::array<char, 1024> recv_buf {};
		std::iota(buf.begin(), buf.end(), 0);

		auto acceptor = std::unique_ptr<IoAcceptor>(manager.createServer("inproc://foo"));
		std::thread clientThread([&](){
				auto client = std::unique_ptr<IoLine>(manager.createClient("inproc://foo"));
			});

		bool hasConnectionLoss = false;
		std::thread serverThread([&](){
				auto server = std::unique_ptr<IoLine>(acceptor->waitConnection(100));
				int timeout = 100;
				server->setOption(LineOption::ReceiveTimeout, &timeout);
				ssize_t rc = server->read(recv_buf.data(), recv_buf.size());
				if(rc == eConnectionLost)
					hasConnectionLoss = true;
			});

		clientThread.join();
		serverThread.join();

		REQUIRE(hasConnectionLoss);
	}
}


