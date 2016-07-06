
#include "cppio/iolinemanager.h"

#include <vector>

namespace cppio
{

struct IoLineManager::Impl
{
	std::vector<std::unique_ptr<IoLineFactory>> factories;
};

IoLineManager::IoLineManager() : m_impl(new Impl)
{
}

IoLineManager::~IoLineManager()
{
}

IoLine* IoLineManager::createClient(const std::string& address)
{
	auto delimiter = address.find_first_of("://");
	if(delimiter == std::string::npos)
		return nullptr;

	auto scheme = address.substr(0, delimiter);
	for(const auto& factory : m_impl->factories)
	{
		if(factory->supportsScheme(scheme))
		{
			auto baseAddress = address.substr(delimiter + 3);
			return factory->createClient(baseAddress);
		}
	}
	return nullptr;
}

IoAcceptor* IoLineManager::createServer(const std::string& address)
{
	auto delimiter = address.find_first_of("://");
	if(delimiter == std::string::npos)
		return nullptr;

	auto scheme = address.substr(0, delimiter);
	for(const auto& factory : m_impl->factories)
	{
		if(factory->supportsScheme(scheme))
		{
			auto baseAddress = address.substr(delimiter + 3);
			return factory->createServer(baseAddress);
		}
	}
	return nullptr;
}


void IoLineManager::registerFactory(std::unique_ptr<IoLineFactory> factory)
{
	return m_impl->factories.push_back(std::move(factory));
}

}

