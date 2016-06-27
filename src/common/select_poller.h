
#ifndef COMMON_SELECT_POLLER_H
#define COMMON_SELECT_POLLER_H

#include "cppio/poller.h"

#include <vector>
#include <unordered_map>

#include <sys/select.h>
#include <sys/time.h>

namespace cppio
{
class SelectPoller : public Poller
{
public:
	SelectPoller();
	virtual ~SelectPoller();

	virtual void addLine(const std::shared_ptr<Pollable>& line, LineEvent events);
	virtual void removeLine(const std::shared_ptr<Pollable>& line);

	virtual bool poll(const std::chrono::milliseconds& timeout);
	virtual LineEvent eventsForLine(const std::shared_ptr<Pollable>& line);

private:
	std::vector<std::pair<std::shared_ptr<Pollable>, LineEvent>> m_pollables;
	fd_set m_readfds;
	fd_set m_writefds;
	fd_set m_errorfds;
	std::unordered_map<std::shared_ptr<Pollable>, LineEvent> m_events;
};
}

#endif /* ifndef COMMON_SELECT_POLLER _H*/
