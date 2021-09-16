/*
 * MIT License
 * 
 * Copyright (c) 2021 ETJump team <zero@etjump.com>
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
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

namespace ETJump
{
	ProgressionTrackerParser::ProgressionTrackerParser(const std::string& trackerString) : _originalTrackerString(trackerString), _trackerString(trackerString)
	{
		parse();
	}


	void ProgressionTrackerParser::parse()
	{
		// remove whitespace
		_trackerString.erase(remove_if(begin(_trackerString), end(_trackerString), ::isspace), end(_trackerString));

		// get pairs
		std::vector<std::string> pairs;
		split(pairs, _trackerString, boost::is_any_of("|"));

		_parsedPairs.clear();
		for (const auto & pair : pairs)
		{
			std::vector<std::string> indexValuePair;
			split(indexValuePair, pair, boost::is_any_of(","));

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
				_errors.push_back((boost::format("value \"%s\" is not an integer") % indexValuePair[0]).str());
			}
			catch (const std::out_of_range&)
			{
				_errors.push_back((boost::format("value \"%s\" is out of range") % indexValuePair[0]).str());
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
				_errors.push_back((boost::format("index \"%s\" is not an integer") % indexValuePair[0]).str());
			}
			catch (const std::out_of_range&)
			{
				_errors.push_back((boost::format("index \"%s\" is out of range") % indexValuePair[0]).str());
			}

			if (result.index < 0)
			{
				_errors.push_back((boost::format("index \"%s\" is less than 0") % indexValuePair[0]).str());
			}

			try
			{
				result.value = stoi(indexValuePair[1]);
			}
			catch (const std::invalid_argument&)
			{
				_errors.push_back((boost::format("value \"%s\" is not an integer") % indexValuePair[1]).str());
			}
			catch (const std::out_of_range&)
			{
				_errors.push_back((boost::format("value \"%s\" is out of range") % indexValuePair[1]).str());
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

	ProgressionTrackerParser::~ProgressionTrackerParser()
	{
	}
}
