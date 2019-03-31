#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <functional>
#include <map>
#include <vector>

namespace Utilities
{
	class ResultSetFormatter
	{
	public:
		typedef std::map<std::string, std::string> Row;

		ResultSetFormatter();
		~ResultSetFormatter();

		/**
		 * @param headers Displayed headers. If header does not exist on row, an empty column will be displayed
		 * @param rows Data
		 * @param pageSize 
		 * @param page
		 * @returns the buffer with the data
		 */
		std::string toString(std::vector<std::string> headers, std::vector<Row> rows, int pageSize, int page);
	private:
		unsigned totalWidth(std::map<std::string, unsigned> columnWidths);
	};
}



