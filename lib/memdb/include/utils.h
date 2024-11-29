#pragma once

#include <stdexcept>
#include <string>
#include <iostream>

#include "base.h"
#include "bytes.h"
#include "lexem.h"
#include "value.h"

namespace memdb
{
	inline bool strcmpi(const std::string &a, const std::string &b)
	{
		if (a.length() != b.length())
			return false;

		for (size_t i = 0; i < a.length(); ++i)
		{
			if (tolower(a[i]) != tolower(b[i]))
				return false;
		}

		return true;
	}

	template <typename T>
	inline void write_int(std::ostream &out, T val)
	{
		out.write((const char *)&val, sizeof(val));
	}

	inline void write_string(std::ostream &out, const std::string &val)
	{
		size_t n = val.size();
		write_int(out, n);
		out.write((const char *)val.c_str(), n);
	}

	inline void write_bytes(std::ostream &out, const Bytes &val)
	{
		size_t n = val.size();
		write_int(out, n);
		out.write((const char *)val.data(), n);
	}

	template <typename T>
	inline T read_int(std::istream &in)
	{
		T val;
		in.read((char *)&val, sizeof(val));
		return val;
	}

	inline std::string read_string(std::istream &in)
	{
		size_t n = read_int<size_t>(in);
		char *data = new char[n];
		in.read(data, n);
		std::string s = std::string(data, data + n);
		delete[] data;
		return s;
	}

	inline Bytes read_bytes(std::istream &in)
	{
		size_t n = read_int<size_t>(in);
		uint8_t *data = new uint8_t[n];
		in.read((char *)data, n);
		Bytes b = Bytes(data, data + n);
		delete[] data;
		return b;
	}

	inline Value lex_to_value(const Lexem& lexem)
	{
		if (lexem.type == LexemType::INT_LIT)
		{
			return Value((int32_t)std::stoi(lexem.value));
		}
		if (lexem.type == LexemType::BOOL_LIT)
		{
			return Value((bool)(strcmpi(lexem.value, "true") ? true : false));
		}
		if (lexem.type == LexemType::STR_LIT)
		{
			return Value(lexem.value);
		}
		if (lexem.type == LexemType::BT_LIT)
		{
			return Value((Bytes)bytes_from_hex_string(lexem.value));
		}
		throw std::runtime_error("No conversion");
	}

	inline Op lex_to_op(const Lexem& lexem)
	{
		switch (lexem.type)
		{
		case LexemType::PLUS:
			return Op::PLS;
		case LexemType::MINUS:
			return Op::MNS;
		case LexemType::MULT:
			return Op::MUL;
		case LexemType::DIV:
			return Op::DIV;
		case LexemType::MOD:
			return Op::MOD;
		case LexemType::EQ:
			return Op::EQ;
		case LexemType::NE:
			return Op::NE;
		case LexemType::LT:
			return Op::LT;
		case LexemType::GT:
			return Op::GT;
		case LexemType::LE:
			return Op::LE;
		case LexemType::GE:
			return Op::GE;
		case LexemType::AND:
			return Op::AND;
		case LexemType::OR:
			return Op::OR;
		case LexemType::XOR:
			return Op::XOR;
		case LexemType::NOT:
			return Op::NOT;
		}
		throw std::runtime_error("No conversion");
	}

	inline bool is_logic_op(const Lexem& lex)
	{
		const auto& type = lex.type;
		return type == LexemType::AND ||
			type == LexemType::OR ||
			type == LexemType::XOR;
	}

	inline bool is_rel_op(const Lexem& lex)
	{
		const auto& type = lex.type;
		return 
			type == LexemType::EQ ||
			type == LexemType::NE ||
			type == LexemType::GE ||
			type == LexemType::LE ||
			type == LexemType::GT ||
			type == LexemType::LT;
	}

	inline bool is_math_op(const Lexem& lex)
	{
		const auto& type = lex.type;
		return 
			type == LexemType::PLUS ||
			type == LexemType::MINUS ||
			type == LexemType::MULT ||
			type == LexemType::DIV ||
			type == LexemType::MOD;
	}	

	inline bool is_logic_op(Op op)
	{		
		return 
			op == Op::AND ||
			op == Op::OR ||
			op == Op::XOR;
	}

	inline bool is_rel_op(Op op)
	{		
		return 
			op == Op::EQ ||
			op == Op::NE ||
			op == Op::GE ||
			op == Op::LE ||
			op == Op::GT ||
			op == Op::LT;
	}

	inline bool is_math_op(Op op)
	{		
		return 
			op == Op::PLS ||
			op == Op::MNS ||
			op == Op::MUL ||
			op == Op::DIV ||
			op == Op::MOD;
	}

	inline bool is_literal(const Lexem& lex)
	{
		const auto& type = lex.type;
		return
			type == LexemType::INT_LIT ||
			type == LexemType::BOOL_LIT ||
			type == LexemType::STR_LIT ||
			type == LexemType::BT_LIT;
	}

	inline bool is_id(const Lexem& lex)
	{		
		return lex.type == LexemType::ID;			
	}

	inline RelOp op_to_relop(Op op)
	{
		switch (op)
		{		
		case memdb::Op::EQ:
			return RelOp::EQ;
		case memdb::Op::NE:
			return RelOp::NE;
		case memdb::Op::LT:
			return RelOp::LT;
		case memdb::Op::GT:
			return RelOp::GT;
		case memdb::Op::LE:
			return RelOp::LE;
		case memdb::Op::GE:
			return RelOp::GE;
		}
		throw std::runtime_error("No conversion");
	}
}
