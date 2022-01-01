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

#include "etj_printer.h"
#include "etj_string_utilities.h"

#include "g_local.h"

void Printer::LogPrint(std::string message)
{
	std::string partialMessage;
	while (message.length() > 1000)
	{
		partialMessage = message.substr(0, 1000);
		message        = message.substr(1000);
		G_LogPrintf("%s", partialMessage.c_str());
	}
	if (message.length() > 0)
	{
		G_LogPrintf(message.c_str());
	}
}

void Printer::LogPrintln(const std::string &message)
{
	LogPrint(message + "\n");
}

void Printer::SendConsoleMessage(int clientNum, std::string message)
{
	auto splits = ETJump::splitString(message, '\n', BYTES_PER_PACKET);
	for (auto &split : splits) 
	{
		if (clientNum == CONSOLE_CLIENT_NUMBER) 
		{
			G_Printf("%s", split.c_str());
		}
		else
		{
			trap_SendServerCommand(clientNum, va("print \"%s\"", split.c_str()));
		}
	}
}

void Printer::SendChatMessage(int clientNum, const std::string &message)
{
	if (clientNum == CONSOLE_CLIENT_NUMBER)
	{
		G_Printf("%s", message.c_str());
	}
	else
	{
		trap_SendServerCommand(clientNum, va("chat \"%s\"", message.c_str()));
	}
}

void Printer::SendPopupMessage(int clientNum, const std::string &message)
{
	if (clientNum == CONSOLE_CLIENT_NUMBER)
	{
		G_Printf("%s", message.c_str());
	}
	else
	{
		trap_SendServerCommand(clientNum, va("cpm \"%s\"", message.c_str()));
	}
}

void Printer::BroadcastConsoleMessage(std::string message)
{
	auto splits = ETJump::splitString(message, '\n', BYTES_PER_PACKET);
	for (auto &split : splits) 
	{
		trap_SendServerCommand(-1, va("print \"%s\"", split.c_str()));
		G_Printf("%s", split.c_str());
	}
}

void Printer::BroadcastChatMessage(const std::string &message)
{
	trap_SendServerCommand(-1, va("chat \"%s\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void Printer::BroadcastPopupMessage(const std::string &message)
{
	trap_SendServerCommand(-1, va("cpm \"%s\n\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void Printer::BroadcastLeftBannerMessage(const std::string &message)
{
	trap_SendServerCommand(-1, va("cpm \"%s\n\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void Printer::BroadCastBannerMessage(const std::string& message)
{
	trap_SendServerCommand(-1, va("cpm \"%s\n\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void Printer::SendBannerMessage(int clientNum, const std::string &message)
{
	if (clientNum == CONSOLE_CLIENT_NUMBER)
	{
		G_Printf("%s\n", message.c_str());
	}
	else
	{
		trap_SendServerCommand(clientNum, va("cpm \"%s\n\"", message.c_str()));
	}
}

void Printer::SendCommand(int clientNum, const std::string &command)
{
	trap_SendServerCommand(clientNum, command.c_str());
}

void Printer::BroadcastTopBannerMessage(const std::string& message)
{
	trap_SendServerCommand(-1, va("bp \"%s\n\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void Printer::BroadcastCenterMessage(const std::string& message)
{
	trap_SendServerCommand(-1, va("cp \"%s\n\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void Printer::SendCenterMessage(int clientNum, const std::string& message)
{
	trap_SendServerCommand(clientNum, ETJump::stringFormat("cp \"%s\n\"", message).c_str());
}

void Printer::SendCommandToAll(const std::string& command)
{
	trap_SendServerCommand(-1, command.c_str());
}

void Printer::SendCommand(std::vector<int> clientNums, const std::string &command)
{
	for (auto &clientNum : clientNums)
	{
		trap_SendServerCommand(clientNum, command.c_str());
	}
}
