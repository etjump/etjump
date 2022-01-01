/*
 * MIT License
 * 
 * Copyright (c) 2022 ETJump team <zero@etjump.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "etj_progression_tracker_parser.h"
#include <algorithm>
#include <vector>
#include <string>
#include "etj_string_utilities.h"

namespace ETJump
{
	ProgressionTrackerParser::ProgressionTrackerParser(const std::string& trackerString) : _originalTrackerString(trackerString), _trackerString(trackerString)
	{
		parse();
	}

	ProgressionTrackerParser::~ProgressionTrackerParser()
	{}

	void ProgressionTrackerParser::parse()
	{
		// remove whitespace
		_trackerString.erase(remove_if(begin(_trackerString), end(_trackerString), ::isspace), end(_trackerString));

		// get pairs
		std::vector<std::string> pairs = ETJump::StringUtil::split(_trackerString, "|");

		_parsedPairs.clear();
		for (const auto & pair : pairs)
		{
			std::vector<std::string> indexValuePair = ETJump::StringUtil::split(pair, ",");

			_parsedPairs.push_back(parseIndexValuePair(indexValuePair));
		}
	}

	ProgressionTrackerParser::IndexValuePair ProgressionTrackerParser::parseIndexValuePair(std::vector<std::string> indexValuePair)
	{
		IndexValuePair result{};
		if (indexValuePair.size() == 1)
		{
			result.index = 0;
			try
			{
				result.value = stoi(indexValuePair[0]);
			}
			catch (const std::invalid_argument&)
			{
				_errors.push_back(ETJump::stringFormat("value \"%s\" is not an integer", indexValuePair[0]));
			}
			catch (const std::out_of_range&)
			{
				_errors.push_back(ETJump::stringFormat("value \"%s\" is out of range", indexValuePair[0]));
			}
		}
		else
		{
			try
			{
				result.index = stoi(indexValuePair[0]) - 1;
			}
			catch (const std::invalid_argument&)
			{
				_errors.push_back(ETJump::stringFormat("index \"%s\" is not an integer", indexValuePair[0]));
			}
			catch (const std::out_of_range&)
			{
				_errors.push_back(ETJump::stringFormat("index \"%s\" is out of range", indexValuePair[0]));
			}

			if (result.index < 0)
			{
				_errors.push_back(ETJump::stringFormat("index \"%s\" is less than 0", indexValuePair[0]));
			}

			try
			{
				result.value = stoi(indexValuePair[1]);
			}
			catch (const std::invalid_argument&)
			{
				_errors.push_back(ETJump::stringFormat("value \"%s\" is not an integer", indexValuePair[1]));
			}
			catch (const std::out_of_range&)
			{
				_errors.push_back(ETJump::stringFormat("value \"%s\" is out of range", indexValuePair[1]));
			}
		}
		return result;
	}

	std::vector<ProgressionTrackerParser::IndexValuePair> ProgressionTrackerParser::getParsedPairs() const
	{
		if (_errors.size())
		{
			throw "Could not parse index-value pairs.";
		}

		return _parsedPairs;
	}

	std::vector<std::string> ProgressionTrackerParser::getErrors() const
	{
		return _errors;
	}
}
