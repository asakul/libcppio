
#ifndef POLLER_H
#define POLLER_H

#include "cppio/ioline.h"

#include <chrono>

namespace cppio
{
enum class LineEvent : int
{
	None = 0,
	Read = (1 << 0),
	Write = (1 << 1),
	Error = (1 << 2)
};

inline LineEvent operator|(LineEvent lhs, LineEvent rhs)
{
	return static_cast<LineEvent>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline LineEvent operator|=(LineEvent& lhs, LineEvent rhs)
{
	lhs = static_cast<LineEvent>(static_cast<int>(lhs) | static_cast<int>(rhs));
	return lhs;
}

inline LineEvent operator&(LineEvent lhs, LineEvent rhs)
{
	return static_cast<LineEvent>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

inline LineEvent operator&=(LineEvent& lhs, LineEvent rhs)
{
	lhs = static_cast<LineEvent>(static_cast<int>(lhs) & static_cast<int>(rhs));
	return lhs;
}

class Poller
{
public:
	virtual ~Poller() = 0;

	virtual void addLine(const std::shared_ptr<Pollable>& line, LineEvent events) = 0;
	virtual void removeLine(const std::shared_ptr<Pollable>& line) = 0;

	virtual bool poll(const std::chrono::milliseconds& timeout) = 0;
	virtual LineEvent eventsForLine(const std::shared_ptr<Pollable>& line) = 0;
};

inline Poller::~Poller() {}
}

#endif /* ifndef POLLER_H */
