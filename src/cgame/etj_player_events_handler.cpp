/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <haapanen.jussi@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "etj_player_events_handler.h"
#include <boost/algorithm/string/case_conv.hpp>

ETJump::PlayerEventsHandler::PlayerEventsHandler()
{
}

ETJump::PlayerEventsHandler::~PlayerEventsHandler()
{
	_callbacks.clear();
}

bool ETJump::PlayerEventsHandler::check(const std::string& event, const std::vector<std::string>& arguments)
{
	auto lowerEvent = boost::algorithm::to_lower_copy(event);
	auto match = _callbacks.find(lowerEvent);
	if (match != end(_callbacks))
	{
		for (auto listener : match->second) listener(arguments);
		return true;
	}
	return false;
}

bool ETJump::PlayerEventsHandler::subscribe(const std::string& event,
std::function<void(const std::vector<std::string>&)> callback)
{
	auto lowerEvent = boost::algorithm::to_lower_copy(event);
	_callbacks[lowerEvent].push_back(callback);
	return true;
}

bool ETJump::PlayerEventsHandler::unsubscribe(const std::string& event)
{
	auto lowerEvent = boost::algorithm::to_lower_copy(event);
	auto listeners = _callbacks.find(lowerEvent);
	if (listeners == end(_callbacks)) return false;
	_callbacks.erase(listeners);
	return true;
}
