/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <zero@etjump.com>
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

#include "etj_cvar_update_handler.h"
#include <boost/algorithm/string/case_conv.hpp>

ETJump::CvarUpdateHandler::CvarUpdateHandler()
{
	callbacks.clear();
}

ETJump::CvarUpdateHandler::~CvarUpdateHandler() {}

bool ETJump::CvarUpdateHandler::check(const vmCvar_t *cvar)
{
	auto match = callbacks.find(cvar->handle);
	if (match != end(callbacks))
	{
		for (auto callback : match->second)
		{
			callback(cvar);
		}
		return true;
	}
	return false;
}

bool ETJump::CvarUpdateHandler::subscribe(const vmCvar_t *target, std::function<void(const vmCvar_t *cvar)> callback)
{
	callbacks[target->handle].push_back(callback);
	return true;
}


bool ETJump::CvarUpdateHandler::unsubscribe(const vmCvar_t *target)
{
	auto callback = callbacks.find(target->handle);
	if (callback != end(callbacks))
	{
		return false;
	}
	callbacks.erase(callback);
	return true;
}



