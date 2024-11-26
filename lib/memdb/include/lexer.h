#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <cctype>

#include "utils.h"

namespace memdb
{

	enum class LexemType
	{
		CREATE,
		TABLE,
		UNIQUE,
		AUTO,
		KEY,
		INSERT,
		TO,
		SELECT,
		FROM,
		WHERE,
		SET,
		UPDATE,
		DELETE,
		JOIN,
		INDEX,
		ON,
		BY,
		ORDERED,
		UNORDERED,
		INT32,
		BOOL,
		STRING,
		BYTES,
		PLUS,
		MINUS,
		MULT,
		DIV,
		MOD,
		EQ,
		NE,
		LT,
		GT,
		LE,
		GE,
		AND,
		NOT,
		OR,
		XOR,
		PIPE,
		LPAR,
		RPAR,
		LBRC,
		RBRC,
		LBRK,
		RBRK,
		DOT,
		COMMA,
		COLON,
		INT_LIT,
		BOOL_LIT,
		STR_LIT,
		BT_LIT,
		ID,
		EOQ // End Of Query
	};

	// Whitespace characters
	std::set<char> WS = {' ', '\t', '\r', '\n'};
	// Punctuation characters
	std::set<char> PUNCT = {
		'.', ',', ':', '{',
		'}', '(', ')', '[',
		']', '<', '>', '=',
		'!', '&', '|', '^',
		'+', '-', '*', '/',
		'%'};

	std::map<std::string, LexemType> RESERVED = {
		{"create", LexemType::CREATE},
		{"table", LexemType::TABLE},
		{"unique", LexemType::UNIQUE},
		{"autoincrement", LexemType::AUTO},
		{"key", LexemType::KEY},
		{"insert", LexemType::INSERT},
		{"to", LexemType::TO},
		{"select", LexemType::SELECT},
		{"from", LexemType::FROM},
		{"where", LexemType::WHERE},
		{"update", LexemType::UPDATE},
		{"set", LexemType::SET},
		{"delete", LexemType::DELETE},
		{"join", LexemType::JOIN},
		{"index", LexemType::INDEX},
		{"on", LexemType::ON},
		{"by", LexemType::BY},
		{"ordered", LexemType::ORDERED},
		{"unordered", LexemType::UNORDERED},
		{"int32", LexemType::INT32},
		{"bool", LexemType::BOOL},
		{"string", LexemType::STRING},
		{"bytes", LexemType::BYTES},
		{"true", LexemType::BOOL_LIT},
		{"false", LexemType::BOOL_LIT}};

	struct Lexem
	{
		LexemType type;
		std::string value;
		size_t line;
		size_t col;
		size_t begin;
		size_t end;
	};

	class LexicalError : public std::exception
	{
		size_t line;
		size_t col;

	public:
		LexicalError(size_t line, size_t col) : std::exception(), line(line), col(col) {}
		virtual const char *what() const noexcept
		{
			std::ostringstream oss;
			oss << "Lexical error at line " << line << " position " << col;
			return oss.str().c_str();
		}
	};

	class Lexer
	{
		size_t pos = 0;
		size_t line = 1;
		size_t col = 1;
		const std::string &s;

		static bool is_hex_digit(char c)
		{
			return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
		}

		void skip_ws()
		{
			// Skip whitespace characters
			while (pos < s.size() && WS.count(s[pos]) > 0)
			{
				if (s[pos] == '\n')
				{
					line += 1;
					col = 1;
				}
				else
				{
					col += 1;
				}
				pos++;
			}
		}

		void scan_punct(Lexem &lexem)
		{
			lexem.type = LexemType::EOQ; // to suppress compiler warning
			const char c = s[pos];
			switch (c)
			{
			case '.':
				lexem.type = LexemType::DOT;
				break;
			case ',':
				lexem.type = LexemType::COMMA;
				break;
			case ':':
				lexem.type = LexemType::COLON;
				break;
			case '{':
				lexem.type = LexemType::LBRC;
				break;
			case '}':
				lexem.type = LexemType::RBRC;
				break;
			case '(':
				lexem.type = LexemType::LPAR;
				break;
			case ')':
				lexem.type = LexemType::RPAR;
				break;
			case '[':
				lexem.type = LexemType::LBRK;
				break;
			case ']':
				lexem.type = LexemType::RBRK;
				break;
			case '+':
				lexem.type = LexemType::PLUS;
				break;
			case '-':
				lexem.type = LexemType::MINUS;
				break;
			case '*':
				lexem.type = LexemType::MULT;
				break;
			case '/':
				lexem.type = LexemType::DIV;
				break;
			case '%':
				lexem.type = LexemType::MOD;
				break;
			case '=':
				lexem.type = LexemType::EQ;
				break;
			case '<':
				if (pos + 1 < s.size() && s[pos + 1] == '=')
					lexem.type = LexemType::LE;
				else
					lexem.type = LexemType::LT;
				break;
			case '>':
				if (pos + 1 < s.size() && s[pos + 1] == '=')
					lexem.type = LexemType::GE;
				else
					lexem.type = LexemType::GT;
				break;
			case '!':
				if (pos + 1 < s.size() && s[pos + 1] == '=')
					lexem.type = LexemType::NE;
				else
					lexem.type = LexemType::NOT;
				break;
			case '&':
				if (pos + 1 < s.size() && s[pos + 1] == '&')
					lexem.type = LexemType::AND;
				else
					throw LexicalError(line, col);
				break;
			case '|':
				if (pos + 1 < s.size() && s[pos + 1] == '|')
					lexem.type = LexemType::OR;
				else
					lexem.type = LexemType::PIPE;
				break;
			case '^':
				if (pos + 1 < s.size() && s[pos + 1] == '^')
					lexem.type = LexemType::XOR;
				else
					throw LexicalError(line, col);
				break;
			}

			if (lexem.type == LexemType::LE ||
				lexem.type == LexemType::GE ||
				lexem.type == LexemType::NE ||
				lexem.type == LexemType::AND ||
				lexem.type == LexemType::OR ||
				lexem.type == LexemType::XOR)
			{
				lexem.end = pos + 2;
				lexem.value = s.substr(pos, 2);
			}
			else
			{
				lexem.end = pos + 1;
				lexem.value = s.substr(pos, 1);
			}
		}

		void scan_id(Lexem &lexem)
		{
			size_t p = pos;
			while (p < s.size() && (isalnum(s[p]) || s[p] == '_'))
			{
				p += 1;
			}
			lexem.end = p;
			lexem.value = s.substr(lexem.begin, lexem.end - lexem.begin);

			bool is_keyword = false;
			for (const auto &kv : RESERVED)
			{
				const std::string &keyword = kv.first;
				if (strcmpi(keyword, lexem.value))
				{
					lexem.type = kv.second;
					is_keyword = true;
					break;
				}
			}
			if (!is_keyword)
			{
				lexem.type = LexemType::ID;
			}
		}

		void scan_number(Lexem &lexem)
		{
			if (pos + 1 < s.size() && s[pos] == '0' && s[pos + 1] == 'x')
			{
				// bytes literal
				size_t p = pos + 2;
				while (p < s.size() && is_hex_digit(s[p]))
				{
					p++;
				}
				if (p < s.size() && WS.count(s[p]) == 0 && PUNCT.count(s[p]) == 0)
					throw LexicalError(line, col);
				lexem.end = p;
				lexem.value = s.substr(lexem.begin, lexem.end - lexem.begin);
				lexem.type = LexemType::BT_LIT;
			}
			else
			{
				// int32 literal or just number
				size_t p = pos + 1;
				while (p < s.size() && isdigit(s[p]))
				{
					p++;
				}
				if (p < s.size() && WS.count(s[p]) == 0 && PUNCT.count(s[p]) == 0)
					throw LexicalError(line, col);
				lexem.end = p;
				lexem.value = s.substr(lexem.begin, lexem.end - lexem.begin);
				lexem.type = LexemType::INT_LIT;
				if (lexem.value.length() > 1 && lexem.value.front() == '0')
					throw LexicalError(line, col); // Leading zero(es) not allowed
			}
		}

		void scan_string_literal(Lexem &lexem)
		{
			size_t p = pos + 1;
			while (p < s.size())
			{
				if (s[p] == '\"' && s[p - 1] != '\\')
					break;

				if (s[p] == '\n')
				{
					line += 1;
					col = 1;
				}
				else
				{
					col += 1;
				}

				p++;
			}
			if (p == s.size())
			{
				throw LexicalError(line, col); // Unclosed "
			}
			lexem.begin += 1;
			lexem.end = p;
			lexem.value = s.substr(lexem.begin, lexem.end - lexem.begin);
			lexem.type = LexemType::STR_LIT;
			pos = p + 1;
		}

	public:
		Lexer(const std::string &s) : s(s) {}

		std::vector<Lexem> tokenize()
		{
			std::vector<Lexem> lexems;
			skip_ws();
			while (pos < s.size())
			{
				const char c = s[pos];
				Lexem lexem;
				lexem.begin = pos;
				lexem.line = line;
				lexem.col = col;

				if (PUNCT.count(c) > 0)
				{
					scan_punct(lexem);
				}
				else if (isalpha(c) || c == '_')
				{
					scan_id(lexem);
				}
				else if (isdigit(c))
				{
					scan_number(lexem);
				}
				else if (c == '\"')
				{
					scan_string_literal(lexem);
				}
				else
				{
					throw LexicalError(line, col);
				}

				if (lexem.type != LexemType::STR_LIT)
				{
					pos += lexem.end - lexem.begin;
					col += lexem.end - lexem.begin;
				}
				lexems.push_back(lexem);

				skip_ws();
			}

			Lexem lexem;
			lexem.type = LexemType::EOQ;
			lexem.begin = lexem.end = pos;
			lexem.line = line;
			lexem.col = col;
			lexems.push_back(lexem);

			return lexems;
		}
	};

}
