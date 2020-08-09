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

#include "etj_user.h"
#include <boost/algorithm/string/join.hpp>
#include "utilities.hpp"

User_s::User_s()
{

}

User_s::User_s(unsigned id, std::string const& guid, std::string const& name, std::string const& hwid)
	:
	id(id),
	guid(guid),
	level(0),
	lastSeen(0),
	name(name),
	updated(0)
{
}

User_s::User_s(unsigned id, std::string const& guid, int level, unsigned lastSeen, std::string const& name, std::string const& hwid, std::string const& title, std::string const& commands, std::string const& greeting)
	: id(id), guid(guid), level(level), lastSeen(lastSeen), name(name), title(title), commands(commands), greeting(greeting), updated(0)
{
}

unsigned User_s::GetId()
{
	return id;
}

std::string User_s::GetGuid()
{
	return guid;
}

char const *User_s::ToChar() const
{
	return va("%d %s %d %d %s %s %s %s %s", id, guid.c_str(), level, lastSeen, name.c_str(), (boost::algorithm::join(hwids, ", ")).c_str(), title.c_str(), commands.c_str(), greeting.c_str());
}

std::string User_s::GetLastSeenString() const
{
	return TimeStampToString(lastSeen);
}

std::string User_s::GetLastVisitString() const
{
	time_t t;
	time(&t);
	unsigned now = static_cast<unsigned>(t);

	return TimeStampDifferenceToString(now - lastSeen);
}
