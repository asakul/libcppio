
#include "select_poller.h"

#include <algorithm>

namespace cppio
{
SelectPoller::SelectPoller()
{
}

SelectPoller::~SelectPoller()
{
}

void SelectPoller::addLine(const std::shared_ptr<Pollable>& line, LineEvent events)
{
	if(line->getNativeHandle() == nullptr)
		throw LineIsNotPollable();
	m_pollables.push_back(std::make_pair(line, events));
}

void SelectPoller::removeLine(const std::shared_ptr<Pollable>& line)
{
	auto it = std::find_if(m_pollables.begin(),
			m_pollables.end(),
			[&](const std::pair<std::shared_ptr<Pollable>, LineEvent>& other)
			{
				return other.first == line;
			});
	if(it == m_pollables.end())
		return;
	m_pollables.erase(it);
}

bool SelectPoller::poll(const std::chrono::milliseconds& timeout)
{
	FD_ZERO(&m_readfds);
	FD_ZERO(&m_writefds);
	FD_ZERO(&m_errorfds);

	int max_fd = 0;
	for(const auto& pollable : m_pollables)
	{
		auto line = pollable.first;
		auto events = pollable.second;
		int handle = *static_cast<int*>(line->getNativeHandle());
		if((events & LineEvent::Read) != LineEvent::None)
			FD_SET(handle, &m_readfds);
		if((events & LineEvent::Write) != LineEvent::None)
			FD_SET(handle, &m_writefds);
		if((events & LineEvent::Error) != LineEvent::None)
			FD_SET(handle, &m_errorfds);

		if(handle > max_fd)
			max_fd = handle;
	}

	struct timeval tv;
	auto secs = std::chrono::duration_cast<std::chrono::seconds>(timeout);
	tv.tv_sec = secs.count();
	tv.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(timeout - secs).count();
	int result = select(max_fd + 1, &m_readfds, &m_writefds, &m_errorfds, &tv);

	m_events.clear();
	if(result < 0)
		throw IoException("Polling error, errno = " + std::to_string(errno));

	else if(result > 0)
	{
		for(size_t i = 0; i < m_pollables.size(); i++)
		{
			auto line = m_pollables[i].first;
			int* handle = static_cast<int*>(line->getNativeHandle());
			LineEvent events = LineEvent::None;
			if(FD_ISSET(*handle, &m_readfds))
			{
				events |= LineEvent::Read;
			}
			if(FD_ISSET(*handle, &m_writefds))
			{
				events |= LineEvent::Write;
			}
			if(FD_ISSET(*handle, &m_errorfds))
			{
				events |= LineEvent::Error;
			}
			m_events[line] = events;
		}
		return true;
	}
	return false;
}

LineEvent SelectPoller::eventsForLine(const std::shared_ptr<Pollable>& line)
{
	auto it = m_events.find(line);
	if(it != m_events.end())
	{
		return it->second;
	}
	return LineEvent::None;
}

}

