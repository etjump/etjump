#pragma once
#include <string>
#include <vector>
#include <sstream>

namespace ETJump
{
	class File
	{
	public:
		class FileNotFoundException : public std::runtime_error
		{
		public:
			explicit FileNotFoundException(const std::string& message)
				: runtime_error(message)
			{
			}
		};

		class WriteFailedException : public std::runtime_error
		{
		public:
			explicit WriteFailedException(const std::string& message)
				: runtime_error(message)
			{
			}
		};

		class PathNotFoundException : public std::runtime_error
		{
		public:
			explicit PathNotFoundException(const std::string& message)
				: runtime_error(message)
			{
			}
		};

		typedef int FileHandle;
		enum class Mode
		{
			Read,
			Write,
			Append,
			AppendSync
		};

		static const int FILE_NOT_FOUND = -1;
		static const int INVALID_FILE_HANDLE = 0;
		static const int READ_ALL_BYTES = -1;

		// throws FileNotFoundException if the mode is read and there's no 
		// such file.
		// Write will create the file in that case
		explicit File(const std::string& path, Mode mode = Mode::Read);
		~File();

		// reads `bytes` bytes from the file.
		// if file length < bytes, reads length bytes
		std::vector<char> read(int bytes = READ_ALL_BYTES);

		// writes all data to the file
		// throws std::logic_error if mode is Read
		// throws WriteFailedException if written bytes count != data bytes count
		void write(const std::string& data) const;

		// lists files in a dir with extension
		// extension should include dot (not just ext) e.g. ".bsp"
		// NOTE: Only gets files from referenced pk3s, not from local directory
		// use localFileList function to get files from local directory
		static std::vector<std::string> fileList(const std::string& path, const std::string& extension);

		// lists file in a dir with extensions
		// extension should include a dot e.g. ".bsp"
		// throws PathNotFoundException if path is not found.
		static std::vector<std::string> localFileList(const std::string& path, const std::string& extension);

		// builds the quake file system path from the file name
		static std::string getPath(const std::string file);
	private:
		std::string _path;
		FileHandle _handle;
		int _length;
		Mode _mode;
	};
}



