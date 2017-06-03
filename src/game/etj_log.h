#pragma once
#include <string>

namespace ETJump
{
	class Log
	{
	public:
		enum class LogLevel
		{
			Debug,
			Info,
			Warn,
			Error,
			Fatal
		};
		explicit Log(const std::string& name);
		~Log();

		// arbitrary limit found out by testing
		static const int MAX_SERVER_PRINT_LEN = 1013;

		void debug(const std::string& text);
		void debugLn(const std::string& text);
		void info(const std::string& text);
		void infoLn(const std::string& text);
		void warn(const std::string& text);
		void warnLn(const std::string& text);
		void error(const std::string& text);
		void errorLn(const std::string& text);
		void fatal(const std::string& text);
		void fatalLn(const std::string& text);

		static void setLogLevel(LogLevel level);
		static LogLevel getLogLevel(LogLevel level);

	private:
		void log(const std::string& text, LogLevel level);
		static std::string toString(LogLevel level);

		static LogLevel _level;
		std::string _name;
	};
}
