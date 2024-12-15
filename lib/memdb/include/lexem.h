#pragma once

#include <string>

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

	struct Lexem
	{
		LexemType type = LexemType::EOQ;
		std::string value;
		size_t line = 0;
		size_t col = 0;
		size_t begin = 0;
		size_t end = 0;		
	};	
}