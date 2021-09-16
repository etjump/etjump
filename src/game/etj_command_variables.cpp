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

#include "etj_console_variables.h"

#include "g_local.h"

std::string ETJump::ConsoleVariables::getCvarString(const std::string& name)
{
	char buffer[MAX_CVAR_VALUE_STRING];
	trap_Cvar_VariableStringBuffer(name.c_str(), buffer, sizeof(buffer));
	return buffer;
}

int ETJump::ConsoleVariables::getCvarInteger(const std::string& name)
{
	return trap_Cvar_VariableIntegerValue(name.c_str());
}

//float ETJump::ConsoleVariables::getCvarFloat(const std::string& name)
//{
//	return trap_Cvar_VariableValue(name.c_str());
//}
