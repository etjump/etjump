#pragma once

#include <string>
#include <vector>

namespace ETJump
{
	class ProgressionTrackerParser
	{
	public:
		struct IndexValuePair
		{
			int index;
			int value;
		};

		ProgressionTrackerParser(const std::string& trackerString);
		~ProgressionTrackerParser();

		std::vector<std::string> getErrors() const;
		std::vector<IndexValuePair> getParsedPairs() const;
	private:
		IndexValuePair parseIndexValuePair(std::vector<std::string> indexValuePair);
		void parse();
		std::string _originalTrackerString;
		std::string _trackerString;
		std::vector<std::string> _errors;
		std::vector<IndexValuePair> _parsedPairs;
	};


}

