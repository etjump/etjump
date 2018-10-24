#include <stdexcept>
#include <algorithm>
#include "../game/etj_file.h"
#include "../game/etj_string_utilities.h"
#include "../game/etj_filesystem.h"
#ifdef GAMEDLL
#include "g_local.h"
#elif CGAMEDLL
#include "../cgame/cg_local.h"
#endif

void ETJump::FileSystem::copy(const std::string &src, const std::string &dst)
{
	if (src == dst) return;
	File srcFile(src, File::Mode::Read);
	File dstFile(dst, File::Mode::Write);
	dstFile.write(srcFile.read());
}

void ETJump::FileSystem::move(const std::string &src, const std::string &dst)
{
	copy(src, dst);
	remove(src);
}

bool ETJump::FileSystem::remove(const std::string &path)
{
#ifdef CGAMEDLL
	int success = trap_FS_Delete(path.c_str());
	return success == 1;
#elif GAMEDLL
	// hacky fallback
	int success = trap_FS_Rename(path.c_str(), "");
	return true;
#endif
}

bool ETJump::FileSystem::exists(const std::string &path)
{
	int handle;
	int length = trap_FS_FOpenFile(path.c_str(), &handle, FS_READ);
	return length != File::FILE_NOT_FOUND;
}

bool ETJump::FileSystem::safeCopy(const std::string &src, const std::string &dst)
{
	try
	{
		copy(src, dst);
	}
	catch (File::FileNotFoundException &err)
	{
		return false;
	}
	catch (File::WriteFailedException &err)
	{
		return false;
	}
	catch (std::logic_error &err)
	{
		return false;
	}

	return true;
}

bool ETJump::FileSystem::safeMove(const std::string &src, const std::string &dst)
{
	return safeCopy(src, dst) && remove(src);
}

std::vector<std::string> ETJump::FileSystem::getFileList(const std::string &path, const std::string &ext)
{
	const int BUFF_SIZE = 200000;
	auto demoList = std::unique_ptr<char[]>(new char[BUFF_SIZE]);
	auto numFiles = trap_FS_GetFileList(path.c_str(), ext.c_str(), demoList.get(), BUFF_SIZE);
	std::vector<std::string> files;
	auto namePtr = demoList.get();
	for (auto i = 0; i < numFiles; ++i)
	{
		files.push_back(std::string(namePtr));
		namePtr += strlen(namePtr) + 1;
	}
	return files;
}

std::string ETJump::FileSystem::Path::sanitize(std::string path)
{
	return sanitizeFile(path);
}

std::string ETJump::FileSystem::Path::sanitizeFile(std::string path)
{
	static const char illegalChars[] = "\\/:*?\"<>.|";
	for (char ch : illegalChars)
	{
		std::replace(path.begin(), path.end(), ch, '_');
	}
	return path;
}

std::string ETJump::FileSystem::Path::sanitizeFolder(std::string path)
{
	static const char illegalChars[] = "\\:*?\"<>.|";
	for (char ch : illegalChars)
	{
		std::replace(path.begin(), path.end(), ch, '_');
	}
	return path;
}
