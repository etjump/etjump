#include "etj_file.h"
#include <boost/format.hpp>
#ifdef GAMEDLL
#include "g_local.h"
#elif CGAMEDLL
#include "../cgame/cg_local.h"
#endif

#ifdef Rectangle
#undef Rectangle
#endif

#include "../tinydir/tinydir.h"

ETJump::File::File(const std::string& path, Mode mode): _path(path), _handle(INVALID_FILE_HANDLE), _mode(mode)
{
	fsMode_t fsMode;
	switch (_mode)
	{
	case Mode::Read: 
		fsMode = FS_READ;
		break;
	case Mode::Write: 
		fsMode = FS_WRITE;
		break;
	case Mode::Append: 
		fsMode = FS_APPEND;
		break;
	case Mode::AppendSync: 
		fsMode = FS_APPEND_SYNC;
		break;
	default: 
		fsMode = FS_READ;
		break;
	}

	_length = trap_FS_FOpenFile(_path.c_str(), &_handle, fsMode);
	if (_mode == Mode::Read && _length == FILE_NOT_FOUND)
	{
		throw FileNotFoundException(_path);
	}
}

ETJump::File::~File()
{
	if (_handle > INVALID_FILE_HANDLE)
	{
		trap_FS_FCloseFile(_handle);
	}
}

std::vector<char> ETJump::File::read(int bytes)
{
	if (_mode != Mode::Read)
	{
		throw std::logic_error("Cannot read from a file when mode is not Mode::Read.");
	}
	auto readBytes = bytes == READ_ALL_BYTES ? _length : (_length >= bytes ? bytes : _length);
	auto buffer = std::vector<char>(readBytes);
	
	trap_FS_Read(buffer.data(), readBytes, _handle);
	return move(buffer);
}

void ETJump::File::write(const std::string& data) const
{
	if (_mode == Mode::Read)
	{
		throw std::logic_error("Cannot write to a file when mode is Mode::Read.");
	}

	auto bytesWritten = trap_FS_Write(data.c_str(), data.length(), _handle);
	if (bytesWritten != data.length())
	{
		throw WriteFailedException((boost::format("Write to file %s failed. Wrote %d out of %d bytes.")
			% _path % bytesWritten % data.length()).str());
	}
}

std::vector<std::string> ETJump::File::fileList(const std::string& path, const std::string& extension)
{
	std::vector<std::string> files;
	char buffer[1 << 16] = "";
	auto numDirs = trap_FS_GetFileList(path.c_str(), extension.c_str(), buffer, sizeof(buffer));
	auto dirPtr = buffer;
	auto dirLen = 0;

	for (auto i = 0; i < numDirs; i++, dirPtr += dirLen + 1)
	{
		dirLen = strlen(dirPtr);
		if (strlen(dirPtr) > 4)
		{
			dirPtr[strlen(dirPtr) - 4] = 0;
		}

		char file[MAX_QPATH] = "";
		Q_strncpyz(file, dirPtr, sizeof(file));
		files.push_back(file);
	}
	return move(files);
}

std::vector<std::string> ETJump::File::localFileList(const std::string& path, const std::string& extension)
{
	tinydir_dir dir;

	auto realPath = getPath(path);

	if (tinydir_open_sorted(&dir, realPath.c_str()) == -1)
	{
		throw PathNotFoundException(path);
	}

	std::vector<std::string> files;
	for (auto i = 0; i < dir.n_files; ++i)
	{
		tinydir_file file;
		if (tinydir_readfile_n(&dir, &file, i) == -1)
		{
			throw std::runtime_error("Unknown runtime error while listing directory contents.");
		}

		if (file.is_dir)
		{
			continue;
		}

		files.push_back(file.name);
	}
	return files;
}

std::string ETJump::File::getPath(const std::string file)
{
	char game[MAX_CVAR_VALUE_STRING] = "";
	char base[MAX_CVAR_VALUE_STRING] = "";
	trap_Cvar_VariableStringBuffer("fs_game", game, sizeof(game));
	trap_Cvar_VariableStringBuffer("fs_homepath", base, sizeof(base));

	auto path = (boost::format("%s/%s/%s") % base % game % file).str();
	for (auto & c : path)
	{
		if (c == '/' || c == '\\')
		{
			c = PATH_SEP;
		}
	}
	return move(path);
}

