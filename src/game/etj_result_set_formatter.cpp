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

#include "etj_result_set_formatter.h"
#include <algorithm>
#include <numeric>
#include "etj_string_utilities.h"

Utilities::ResultSetFormatter::ResultSetFormatter()
{
}

Utilities::ResultSetFormatter::~ResultSetFormatter()
{
}

std::string Utilities::ResultSetFormatter::toString(std::vector<std::string> headers, std::vector<std::map<std::string, std::string>> rows, int pageSize, int page)
{
	if (rows.size() == 0)
	{
		return "";
	}
	std::map<std::string, unsigned> maxColumnWidths;
	for (const auto & header : headers)
	{
		maxColumnWidths[header] = header.length();
	}

	const unsigned maxChars = 80;
	for (int rowIdx = 0, rowLength = rows.size(); rowIdx < rowLength; ++rowIdx)
	{
		auto row = rows[rowIdx];

		for (const auto & header : headers)
		{
			auto iter = row.find(header);
			if (iter != end(row))
			{
                maxColumnWidths[header] = std::max(
                            std::max(maxColumnWidths[header],
                                static_cast<unsigned int>(header.length())),
                                    static_cast<unsigned int>(iter->second.length()));
				if (maxColumnWidths[header] > maxChars)
				{
					maxColumnWidths[header] = maxChars;
				}
			} else
			{
                maxColumnWidths[header] = std::max(
                            maxColumnWidths[header],
                                static_cast<unsigned int>(std::to_string(rows.size()).size()));
			}
		}
	}

	std::string buffer;
	for (const auto & header : headers)
	{
		buffer += header + std::string(maxColumnWidths[header] - header.length(), ' ') + "  ";
	}
	buffer += "\n";

	for (const auto & header : headers)
	{
		buffer += std::string(header.length(), '-') + std::string(maxColumnWidths[header] - header.length() + 2, ' ');
	}
	buffer += "\n";

	int current = 0;
	int startingIndex = pageSize * page;
	int endingIndex = pageSize * (page + 1);
	for (const auto & row : rows)
	{
		if (current < startingIndex)
		{
			++current;
			continue;
		}
		if (current >= endingIndex)
		{
			break;
		}
		for (const auto & header : headers)
		{
			const auto iter = row.find(header);
			if (iter == end(row))
			{
				auto lwrCase = ETJump::StringUtil::toLowerCase(header);
				if (lwrCase == "index" || lwrCase == "idx" || lwrCase == "i")
				{
                    maxColumnWidths[header] = std::max(
                                maxColumnWidths[header], static_cast<unsigned int>(header.length()));
					auto index = std::to_string(current + 1);
					buffer += index + std::string(maxColumnWidths[header] - index.length() + 2, ' ');
				} else
				{
					buffer += std::string(maxColumnWidths[header] + 2, ' ');
				}
			} else
			{
				auto toBePrinted = iter->second;
				if (iter->second.length() > maxChars)
				{
					toBePrinted = iter->second.substr(0, maxChars);
					toBePrinted[maxChars - 3] = '.';
					toBePrinted[maxChars - 2] = '.';
					toBePrinted[maxChars - 1] = '.';
				}
				
				buffer += toBePrinted + std::string(maxColumnWidths[iter->first] - toBePrinted.length() + 2, ' ');
			}
		}
		buffer += "\n";
		++current;
	}

	return buffer;
}

unsigned Utilities::ResultSetFormatter::totalWidth(std::map<std::string, unsigned> columnWidths)
{
	auto sum = 0;
	for (const auto & column : columnWidths)
	{
		sum += column.second;
	}
	return sum + (columnWidths.size() - 1) * 2;
}
