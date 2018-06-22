//
// Created by Jussi on 5.4.2015.
//

#include "etj_printer.h"
#include <boost/format.hpp>

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
	std::string partialMessage;
	const auto  BYTES_PER_PACKET = 998;
	while (message.length() > BYTES_PER_PACKET)
	{
		partialMessage = message.substr(0, BYTES_PER_PACKET);
		message        = message.substr(BYTES_PER_PACKET);
		if (clientNum == CONSOLE_CLIENT_NUMBER)
		{
			G_Printf("%s", partialMessage.c_str());
		}
		else
		{
			trap_SendServerCommand(clientNum, va("print \"%s\"", partialMessage.c_str()));
		}
	}
	if (message.length() > 0)
	{
		if (clientNum == CONSOLE_CLIENT_NUMBER)
		{
			G_Printf("%s", partialMessage.c_str());
		}
		else
		{
			trap_SendServerCommand(clientNum, va("print \"%s\"", message.c_str()));
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

void Printer::SendLeftMessage(int clientNum, const std::string &message)
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
	std::string partialMessage;
	while (message.length() > 1000)
	{
		partialMessage = message.substr(0, 1000);
		message        = message.substr(1000);

		G_Printf("%s", partialMessage.c_str());
		trap_SendServerCommand(-1, va("print \"%s\"", partialMessage.c_str()));
	}

	if (message.length() > 0)
	{
		G_Printf("%s", message.c_str());
		trap_SendServerCommand(-1, va("print \"%s\"", message.c_str()));
	}
}

void Printer::BroadcastChatMessage(const std::string &message)
{
	trap_SendServerCommand(-1, va("chat \"%s\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void Printer::BroadcastLeftMessage(const std::string &message)
{
	trap_SendServerCommand(-1, va("cpm \"%s\n\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void Printer::BroadcastLeftBannerMessage(const std::string &message)
{
	trap_SendServerCommand(-1, va("cpm \"%s\n\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void Printer::BroadCastBannerMessage(const boost::format &fmt)
{
	trap_SendServerCommand(-1, va("cpm \"%s\n\"", fmt.str().c_str()));
	G_Printf("%s\n", fmt.str().c_str());
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
	trap_SendServerCommand(clientNum, (boost::format("cp \"%s\n\"") % message).str().c_str());
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
