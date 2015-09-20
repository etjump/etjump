//
// Created by Jussi on 5.4.2015.
//

#ifndef ETJUMP_PRINTER_H
#define ETJUMP_PRINTER_H

#include <string>
#include <vector>
#include <boost/format.hpp>

class Printer {
public:
	static const int CONSOLE_CLIENT_NUMBER = -1;
	/**
	 * Prints the message to server console and log. Will
	 * send multiple messages if the message is longer than 1000 bytes
	 * @param message The message to be sent
	 */
	static void LogPrint(std::string message);

	/**
	 * Same functionality as LogPrint but adds a new line in the end
	 * @param message The message to be sent
	 */
	static void LogPrintln(const std::string& message);

	/**
	 * Prints to client console. Will send multiple messages if the
	 * message is longer than 1000 bytes.
	 * If client num is -1 sends to console
	 * @param clientNum The client to send the message to
	 * @param message The message to be sent
	 */
	static void SendConsoleMessage(int clientNum, std::string message);

	/**
	 * Sends a console message to everyone in the server and to server console.
	 * Will send multiple messages if the message is longer than 1000 bytes.
	 * @param message The message to be sent
	 */
	static void BroadcastConsoleMessage(std::string message);

	/**
	 * Prints to client chat. If client num is -1 sends to console
	 * @param clientNum The client to send the message to
	 * @param message The message to be sent
	 */
	static void SendChatMessage(int clientNum, const std::string& message);

	/**
	 * Sends a chat message to everyone in server and to server console.
	 * @param message The message to be sent
	 */
	static void BroadcastChatMessage(const std::string& message);

	/**
	 * Sends a banner message to the client.
	 * @param clientNum the client slot number to send the message to
	 * @param message The message to be sent
	 */
	static void SendBannerMessage(int clientNum, const std::string &message);

	/**
	 * Sends a banner print message to all clients.
	 * @param message The message to be sent
	 */
	static void BroadcastBannerMessage(const std::string& message);
	/**
	* Sends a banner print message to all clients.
	* @param message The message to be sent
	*/
	static void BroadCastBannerMessage(const boost::format& fmt);
	/**
	 * Sends a command to client specified by clientNum. This should probably
	 * be elsewhere but is here for now.
	 * @param clientNum The client to send the message to
	 * @param command The command to be sent
	 */
	static void SendCommand(int clientNum, const std::string& command);

	/**
	 * Sends a command to all clients specified in the clientNums vector.
	 * @param clientNums The list of client numbers to send the command to
	 * @param command The command to send to users
	 */
	static void SendCommand(std::vector<int> clientNums, const std::string& command);

	/**
	* Sends a command to all clients
	* @param command The command to send to users
	*/
	static void SendCommandToAll(const std::string& command);
private:
};


#endif //ETJUMP_PRINTER_H
