#pragma once
#include <string>

namespace ETJump
{
	class ClientAuthentication
	{
	public:
		static const char *guidFile;
		static const int GUID_LEN = 40;
		ClientAuthentication();
		~ClientAuthentication();

		void initialize();
		void authenticate() const;
	private:
		// tries to load the guid. if guid does not exist, returns false
		bool readGuid();
		// creates a new guid
		std::string createGuid() const;
		// tries to save the guid to a file
		bool saveGuid() const;

		std::string _guid;
		std::string _hardwareId;
	};
}



