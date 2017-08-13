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
	static void logPrint(std::string message);

	/**
	 * Same functionality as LogPrint but adds a new line in the end
	 * @param message The message to be sent
	 */
	static void logPrintLn(const std::string& message);

	/**
	 * Prints to client console. Will send multiple messages if the
	 * message is longer than 1000 bytes.
	 * If client num is -1 sends to console
	 * @param clientNum The client to send the message to
	 * @param message The message to be sent
	 */
	static void sendConsoleMessage(int clientNum, std::string message);

	/**
	 * Sends a console message to everyone in the server and to server console.
	 * Will send multiple messages if the message is longer than 1000 bytes.
	 * @param message The message to be sent
	 */
	static void broadcastConsoleMessage(std::string message);

	/**
	 * Prints to client chat. If client num is -1 sends to console
	 * @param clientNum The client to send the message to
	 * @param message The message to be sent
	 */
	static void sendChatMessage(int clientNum, const std::string& message);

	/**
	 * Sends a chat message to everyone in server and to server console.
	 * @param message The message to be sent
	 */
	static void broadcastChatMessage(const std::string& message);

	/**
	 * Sends a banner message to the client.
	 * @param clientNum the client slot number to send the message to
	 * @param message The message to be sent
	 */
	static void sendBannerMessage(int clientNum, const std::string &message);

	/**
	 * Sends a banner print message to all clients.
	 * @param message The message to be sent
	 */
	static void broadcastLeftBannerMessage(const std::string& message);
	/**
	* Sends a banner print message to all clients.
	* @param message The message to be sent
	*/
	static void broadcastBannerMessage(const boost::format& fmt);
	/**
	 * Sends a command to client specified by clientNum. This should probably
	 * be elsewhere but is here for now.
	 * @param clientNum The client to send the message to
	 * @param command The command to be sent
	 */
	static void sendCommand(int clientNum, const std::string& command);

	/**
	 * Sends a command to all clients specified in the clientNums vector.
	 * @param clientNums The list of client numbers to send the command to
	 * @param command The command to send to users
	 */
	static void sendCommand(std::vector<int> clientNums, const std::string& command);

	/**
	* Sends a command to all clients
	* @param command The command to send to users
	*/
	static void sendCommandToAll(const std::string& command);

	/**
	* Broadcasts a top banner message to all clients
	* @param message The message
	*/
	static void broadcastTopBannerMessage(const std::string& message);

	/**
	* Broadcasts a left banner message to all clients
	* @param message The message
	*/
	static void broadcastCenterMessage(const std::string& message);
private:
};


#endif //ETJUMP_PRINTER_H
