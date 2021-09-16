/*
 * MIT License
 * 
 * Copyright (c) 2021 ETJump team <zero@etjump.com>
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

#include "etj_time_utilities.h"
#include <chrono>
#include <time.h>
#include <ctime>

long long ETJump::getCurrentTimestamp()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

ETJump::Clock ETJump::getCurrentClock()
{
	time_t now = std::time(nullptr);
	tm tstruct = *std::localtime(&now);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) % 1000;
	return{ tstruct.tm_hour, tstruct.tm_min, tstruct.tm_sec, (int)ms.count() };
}

ETJump::Clock ETJump::toClock(long long timestamp)
{
	auto hours = timestamp / 6000;
	auto minutes = timestamp / 60000;
	timestamp -= minutes * 60000;
	auto seconds = timestamp / 1000;
	timestamp -= seconds * 1000;
	auto millis = timestamp;

	return{ static_cast<int>(hours), static_cast<int>(minutes), static_cast<int>(seconds), static_cast<int>(millis) };
}

ETJump::Date ETJump::getCurrentDate()
{

	time_t now = std::time(nullptr);
	tm tstruct = *std::localtime(&now);
	return{ tstruct.tm_year + 1900, tstruct.tm_mon + 1, tstruct.tm_mday, tstruct.tm_yday };
}

ETJump::Time ETJump::getCurrentTime()
{
	return{ getCurrentClock(), getCurrentDate() };
}
