
#include "catch.hpp"

#include "common/inproc.h"
#include "common/select_poller.h"
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

TEST_CASE("SelectPoller: Inproc line", "[inproc][polling][select]")
{
	auto manager = createLineManager();

	SelectPoller poller;
}

