#pragma once
#include "etj_level.h"
#include <memory>
#include <map>
#include "etj_log.h"
#include "etj_operation_result.h"

namespace ETJump
{
	class LevelService
	{
	public:
		explicit LevelService(const std::string& levelsFile);
		~LevelService();

		const Level *get(int level);
		OperationResult add(int level, const std::string& name, const std::string& commands, const std::string& greeting);
		OperationResult edit(int level, const LevelChanges& changes, int changedFields);
		OperationResult remove(int level);
		OperationResult readConfig();
		OperationResult writeConfig();
	private:
		std::string getValue(const std::map<std::string, std::string>& dictionary, const std::string& key) const;
		std::string getValue(const std::map<std::string, std::string>& dictionary, const std::vector<std::string>& keys) const;

		std::map<int, Level> _levels;
		const Level _dummyLevel;
		Log _log;
		std::string _levelsFile;
	};
}
