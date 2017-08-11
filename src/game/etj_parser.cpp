#include "etj_parser.h"
#include <locale>


ETJump::Tokenizer::Tokenizer(const std::string& input)
	: _input(input), _inputLength(input.length()), _current(0), _start(0), _end(0), _currentLine(0)
{
}

ETJump::Tokenizer::~Tokenizer()
{
}

ETJump::Tokenizer::Token ETJump::Tokenizer::readLiteral()
{
	auto locale = std::locale::classic();
	while (std::isalpha(_input[_current], locale) || std::isdigit(_input[_current], locale) || std::isspace(_input[_current], locale))
	{
		
	}
}

std::vector<std::string> ETJump::Tokenizer::tokenize()
{
	std::vector<Token> tokens{};

	for (int i = 0; i < _inputLength; ++i)
	{
		switch (_input[i])
		{
		case '\n':
			tokens.push_back(Token(Token::Type::NewLine, _current, _current + 1, _currentLine, _input.substr(_current, 1)));
			++_currentLine;
			break;
		case '/':
			tokens.push_back(Token(Token::Type::Slash, _current, _current + 1, _currentLine, _input.substr(_current, 1)));
			break;
		case '*':
			tokens.push_back(Token(Token::Type::Star, _current, _current + 1, _currentLine, _input.substr(_current, 1)));
			break;
		case '[':
			tokens.push_back(Token(Token::Type::LeftSquareBracket, _current, _current + 1, _currentLine, _input.substr(_current, 1)));
			break;
		case ']':
			tokens.push_back(Token(Token::Type::RightSquareBracket, _current, _current + 1, _currentLine, _input.substr(_current, 1)));
			break;
		default:
			tokens.push_back(readLiteral());
			break;
		}
	}
}

std::string ETJump::Tokenizer::getToken()
{
	while
}
