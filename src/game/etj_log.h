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

		void debug(const std::string& text) const;
		void debugLn(const std::string& text) const;
		void info(const std::string& text) const;
		void infoLn(const std::string& text) const;
		void warn(const std::string& text) const;
		void warnLn(const std::string& text) const;
		void error(const std::string& text) const;
		void errorLn(const std::string& text) const;
		void fatal(const std::string& text) const;
		void fatalLn(const std::string& text) const;

		static void setLogLevel(LogLevel level);
		static LogLevel getLogLevel(LogLevel level);

	private:
		void log(const std::string& text, LogLevel level) const;
		static std::string toString(LogLevel level);

		static LogLevel _level;
		std::string _name;
	};
}
