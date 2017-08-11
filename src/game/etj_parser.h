#pragma once
#include <string>
#include <vector>

namespace ETJump
{
	class Tokenizer
	{
	public:
		struct Token
		{
			enum class Type
			{
				Slash,
				Star,
				LeftSquareBracket,
				RightSquareBracket,
				Literal,
				NewLine
			};

			Token(Type type, unsigned start, unsigned end, unsigned line, const std::string& lexeme): type(type), start(start), end(end), line(line), lexeme(lexeme)
			{
			}

			Type type;
			unsigned start;
			unsigned end;
			unsigned line;
			std::string lexeme;
		};

		explicit Tokenizer(const std::string& input);
		~Tokenizer();

		Token readLiteral();
		std::vector<std::string> tokenize();
	private:
		const std::string _input;
		const unsigned _inputLength;
		unsigned _current;
		unsigned _start;
		unsigned _end;
		unsigned _currentLine;

	};
}



