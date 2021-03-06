
#include "cppio/message.h"

#include "cppio/ioline.h"

#include <cstring>
#include <stdexcept>
#include <cassert>

namespace cppio
{

Frame::Frame()
{
}

Frame::Frame(const void* data, size_t len) : m_data((char*)data, (char*)data + len)
{
}

Frame::Frame(std::vector<char>&& data) : m_data(data)
{
}

Frame Frame::fromValue(uint8_t value)
{
	return Frame(reinterpret_cast<uint8_t*>(&value), sizeof(value));
}

Frame Frame::fromValue(uint16_t value)
{
	return Frame(reinterpret_cast<uint16_t*>(&value), sizeof(value));
}

Frame Frame::fromValue(uint32_t value)
{
	return Frame(reinterpret_cast<uint32_t*>(&value), sizeof(value));
}

Frame Frame::fromValue(const std::string& value)
{
	return Frame(value.data(), value.size());
}

Message::Message()
{
}

Message::~Message()
{
}

void Message::clear()
{
	m_frames.clear();
}

Message Message::readMessage(const void* buffer, size_t bufferLength)
{
	char* start = (char*)buffer;
	char* current = (char*)buffer;
	size_t frames = *((uint32_t*)current);
	current += 4;

	Message msg;

	for(size_t i = 0; i < frames; i++)
	{
		size_t currentFrameLength = *((uint32_t*)current);
		current += 4;
		if(current + currentFrameLength - start > (ssize_t)bufferLength)
			throw std::length_error("Unable to construct Message: end of buffer");

		msg.addFrame(Frame(current, currentFrameLength));
		current += currentFrameLength;
	}

	return msg;
}

void Message::addFrame(const Frame& frame)
{
	m_frames.push_back(frame);
}

void Message::addFrame(Frame&& frame)
{
	m_frames.push_back(frame);
}

size_t Message::messageSize() const
{
	size_t totalSize = 0;
	for(const auto& frame : m_frames)
	{
		totalSize += frame.size() + 4;
	}
	totalSize += 4;
	return totalSize;
}

void Message::writeMessage(void* buffer) const
{
	char* b = (char*)buffer;
	*((uint32_t*)b) = m_frames.size();

	b += 4;
	for(const auto& frame : m_frames)
	{
		*((uint32_t*)b) = frame.size();
		b += 4;
		memcpy(b, frame.data(), frame.size());
		b += frame.size();
	}
}

void Message::get(uint8_t& value, size_t frameNumber) const
{
	value = *reinterpret_cast<const uint8_t*>(frame(frameNumber).data());
}

void Message::get(uint16_t& value, size_t frameNumber) const
{
	value = *reinterpret_cast<const uint16_t*>(frame(frameNumber).data());
}

void Message::get(uint32_t& value, size_t frameNumber) const
{
	value = *reinterpret_cast<const uint32_t*>(frame(frameNumber).data());
}

void Message::get(std::string& value, size_t frameNumber) const
{
	auto f = frame(frameNumber);
	value.assign((const char*)f.data(), f.size());
}

struct MessageProtocol::Impl
{
	IoLine* line;
};

MessageProtocol::MessageProtocol(IoLine* line) : m_impl(new Impl)
{
	m_impl->line = line;
}

MessageProtocol::~MessageProtocol()
{
}

MessageProtocol::MessageProtocol(MessageProtocol&& other) : m_impl(std::move(other.m_impl))
{
}

ssize_t MessageProtocol::readMessage(Message& m)
{
	assert(m.size() == 0);

	uint32_t frames = 0;
	int bytesRead = 0;
	while(bytesRead < 4)
	{
		char* ptr = reinterpret_cast<char*>(&frames) + bytesRead;
		int result = m_impl->line->read(ptr, 4 - bytesRead);
		if(result <= 0)
			return result;
		bytesRead += result;
	}

	for(size_t i = 0; i < frames; i++)
	{
		uint32_t frameLength = 0;
		size_t bytesRead = 0;
		while(bytesRead < 4)
		{
			char* ptr = reinterpret_cast<char*>(&frameLength) + bytesRead;
			int result = m_impl->line->read(ptr, 4 - bytesRead);
			if(result <= 0)
				return result;
			bytesRead += result;
		}

		std::vector<char> data(frameLength);
		bytesRead = 0;

		while(bytesRead < frameLength)
		{
			char* ptr = data.data() + bytesRead;
			int result = m_impl->line->read(ptr, frameLength - bytesRead);
			if(result <= 0)
				return result;
			bytesRead += result;
		}

		m.addFrame(Frame(std::move(data)));
	}
	return 1;
}

ssize_t MessageProtocol::sendMessage(const Message& m)
{
	std::vector<char> buffer(m.messageSize());
	m.writeMessage(buffer.data());
	char* data = buffer.data();
	int towrite = buffer.size();
	while(towrite > 0)
	{
		size_t done = m_impl->line->write(data, towrite);
		towrite -= done;
		data += done;
	}
	return 1;
}


IoLine* MessageProtocol::getLine() const
{
	return m_impl->line;
}


}

