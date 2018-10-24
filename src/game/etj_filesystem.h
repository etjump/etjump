#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <string>
#include <vector>

namespace ETJump
{
	class FileSystem
	{
	public:
		static void copy(const std::string &src, const std::string &dst);
		static void move(const std::string &src, const std::string &dst);
		static bool remove(const std::string &path);
		static bool exists(const std::string &path);
		static bool safeCopy(const std::string &src, const std::string &dst);
		static bool safeMove(const std::string &src, const std::string &dst);
		static std::vector<std::string> getFileList(const std::string &path, const std::string &ext);
		class Path
		{
		public:
			static std::string sanitize(std::string path);
			static std::string sanitizeFile(std::string path);
			static std::string sanitizeFolder(std::string path);
		};
	};
};