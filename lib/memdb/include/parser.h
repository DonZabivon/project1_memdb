#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <cctype>

#include "base.h"
#include "value.h"
#include "column.h"
#include "lexer.h"
#include "utils.h"
#include "ast.h"
#include "visitor.h"

namespace memdb
{
	struct CreateTableDef
	{
		std::string name;
		std::vector<Column> columns;
	};

	struct InsertDef
	{
		std::string name;
		std::vector<Value> values;
		std::map<std::string, Value> named_values;
		bool using_named_values = false;
	};

	struct IndexDef
	{
		std::string name;
		std::vector<std::string> columns;
		bool is_ordered = false;
	};	

	struct SelectDef
	{
		std::string name;
		std::vector<std::string> columns;
		ASTNode *ast;
	};
	
	class Parser
	{
	public:
		Parser(const std::vector<Lexem> &input) : input(input) {}

	protected:
		const std::vector<Lexem> &input;
		size_t pos = 0;

		void syntax_error()
		{
			std::ostringstream oss;
			oss << "Syntax error at line " << input[pos].line << " position " << input[pos].col;
			throw std::runtime_error(oss.str());
		}

		const Lexem &accept(LexemType type)
		{
			if (pos == input.size())
			{
				throw std::runtime_error("Enexpected EOF");
			}
			if (input[pos].type == type)
			{
				return input[pos++];
			}
			else
			{
				syntax_error();
			}
			// unreachable code
			// this reurn just to suppress compiler warning
			return input.back();
		}

		const Lexem &peek()
		{
			return input[pos];
		}
	};	

	class CreateTableParser : public Parser
	{
		CreateTableDef def;
		bool is_unique = false;
		bool is_auto = false;
		bool is_key = false;
		std::string name;
		Type type = Type::NONE;
		uint16_t size = 0;
		Value def_value;

	public:
		CreateTableParser(const std::vector<Lexem>& input) : Parser(input) {}

		CreateTableDef parse()
		{
			accept(LexemType::CREATE);
			accept(LexemType::TABLE);
			def.name = accept(LexemType::ID).value;
			accept(LexemType::LPAR);
			parse_column_def_list();
			accept(LexemType::RPAR);
			accept(LexemType::EOQ);

			return def;
		}

		void parse_column_def_list()
		{
			parse_column_def();
			parse_column_def_list_tail();
		}

		void parse_column_def_list_tail()
		{
			if (type == Type::INT || type == Type::BOOL)
				def.columns.push_back(Column(type, name));
			else
				def.columns.push_back(Column(type, name, size));

			def.columns.back().is_auto = is_auto;
			def.columns.back().is_key = is_key;
			def.columns.back().is_unique = is_unique;
			if (!def_value.is_empty())
			{
				def.columns.back().has_default = true;
				def.columns.back().def_value = def_value;
			}

			if (peek().type == LexemType::COMMA)
			{
				accept(LexemType::COMMA);
				parse_column_def_list();
			}
		}

		void parse_column_def()
		{
			is_unique = false;
			is_auto = false;
			is_key = false;
			def_value = Value();

			parse_column_attr();
			name = accept(LexemType::ID).value;
			accept(LexemType::COLON);
			parse_type();
			parse_def_value();
		}

		void parse_column_attr()
		{
			if (peek().type == LexemType::LBRC)
			{
				accept(LexemType::LBRC);
				parse_column_attr_list();
				accept(LexemType::RBRC);
			}
		}

		void parse_column_attr_list()
		{
			parse_attr();
			parse_column_attr_list_tail();
		}

		void parse_column_attr_list_tail()
		{
			if (peek().type == LexemType::COMMA)
			{
				accept(LexemType::COMMA);
				parse_column_attr_list();
			}
		}

		void parse_attr()
		{
			if (peek().type == LexemType::UNIQUE)
			{
				is_unique = true;
				accept(LexemType::UNIQUE);
			}
			else if (peek().type == LexemType::AUTO)
			{
				is_auto = true;
				accept(LexemType::AUTO);
			}
			else if (peek().type == LexemType::KEY)
			{
				is_key = true;
				accept(LexemType::KEY);
			}
			else
			{
				// Unknown attribute
				syntax_error();
			}
		}

		void parse_type()
		{
			if (peek().type == LexemType::INT32)
			{
				type = Type::INT;
				accept(LexemType::INT32);
			}
			else if (peek().type == LexemType::BOOL)
			{
				type = Type::BOOL;
				accept(LexemType::BOOL);
			}
			else if (peek().type == LexemType::STRING)
			{
				type = Type::STRING;
				parse_str_type();
			}
			else if (peek().type == LexemType::BYTES)
			{
				type = Type::BYTES;
				parse_bytes_type();
			}
			else
			{
				// Unknown type
				syntax_error();
			}
		}

		void parse_str_type()
		{
			accept(LexemType::STRING);
			accept(LexemType::LBRK);
			// TODO: check int value
			size = (uint16_t)std::stoi(accept(LexemType::INT_LIT).value);
			accept(LexemType::RBRK);
		}

		void parse_bytes_type()
		{
			accept(LexemType::BYTES);
			accept(LexemType::LBRK);
			// TODO: check int value
			size = (uint16_t)std::stoi(accept(LexemType::INT_LIT).value);
			accept(LexemType::RBRK);
		}

		void parse_def_value()
		{
			if (peek().type == LexemType::EQ)
			{
				accept(LexemType::EQ);
				if (peek().type == LexemType::INT_LIT)
				{
					def_value = Value(std::stoi(accept(LexemType::INT_LIT).value));
				}
				else if (peek().type == LexemType::BOOL_LIT)
				{
					if (strcmpi(accept(LexemType::BOOL_LIT).value, std::string("true")))
					{
						def_value = Value(true);
					}
					else
					{
						def_value = Value(false);
					}
				}
				else if (peek().type == LexemType::STR_LIT)
				{
					def_value = Value(accept(LexemType::STR_LIT).value);
				}
				else if (peek().type == LexemType::BT_LIT)
				{
					def_value = Value(bytes_from_hex_string(accept(LexemType::STR_LIT).value));
				}
				else
				{
					// Unexpected lexem
					syntax_error();
				}
			}
		}
	};

	class InsertParser : public Parser
	{
		InsertDef def;

	public:
		InsertParser(const std::vector<Lexem>& input) : Parser(input) {}

		InsertDef parse()
		{
			accept(LexemType::INSERT);
			accept(LexemType::LPAR);
			parse_value_list();
			accept(LexemType::RPAR);
			accept(LexemType::TO);
			def.name = accept(LexemType::ID).value;
			accept(LexemType::EOQ);

			return def;
		}

		void parse_value_list()
		{
			if (peek().type == LexemType::ID)
			{
				def.using_named_values = true;
				parse_value_list2();
			}
			else
			{
				parse_value_list1();
			}
		}

		void parse_value_list1()
		{
			parse_value_def1();
			parse_value_list_tail1();
		}

		void parse_value_list_tail1()
		{
			if (peek().type == LexemType::COMMA)
			{
				accept(LexemType::COMMA);
				parse_value_list1();
			}
		}

		void parse_value_def1()
		{
			if (peek().type == LexemType::COMMA || peek().type == LexemType::RPAR)
			{
				def.values.push_back(Value()); // defaul case
			}
			else
			{
				Value value;
				if (peek().type == LexemType::INT_LIT)
				{
					value = Value(std::stoi(accept(LexemType::INT_LIT).value));
				}
				else if (peek().type == LexemType::BOOL_LIT)
				{
					if (strcmpi(accept(LexemType::BOOL_LIT).value, std::string("true")))
					{
						value = Value(true);
					}
					else
					{
						value = Value(false);
					}
				}
				else if (peek().type == LexemType::STR_LIT)
				{
					value = Value(accept(LexemType::STR_LIT).value);
				}
				else if (peek().type == LexemType::BT_LIT)
				{
					value = Value(bytes_from_hex_string(accept(LexemType::BT_LIT).value));
				}
				else
				{
					// Unexpected lexem
					syntax_error();
				}
				def.values.push_back(value);
			}
		}

		void parse_value_list2()
		{
			parse_value_def2();
			parse_value_list_tail2();
		}

		void parse_value_list_tail2()
		{
			if (peek().type == LexemType::COMMA)
			{
				accept(LexemType::COMMA);
				parse_value_list2();
			}
		}

		void parse_value_def2()
		{
			std::string name = accept(LexemType::ID).value;
			accept(LexemType::EQ);
			Value value;
			if (peek().type == LexemType::INT_LIT)
			{
				value = Value(std::stoi(accept(LexemType::INT_LIT).value));
			}
			else if (peek().type == LexemType::BOOL_LIT)
			{
				if (strcmpi(accept(LexemType::BOOL_LIT).value, std::string("true")))
				{
					value = Value(true);
				}
				else
				{
					value = Value(false);
				}
			}
			else if (peek().type == LexemType::STR_LIT)
			{
				value = Value(accept(LexemType::STR_LIT).value);
			}
			else if (peek().type == LexemType::BT_LIT)
			{
				value = Value(bytes_from_hex_string(accept(LexemType::BT_LIT).value));
			}
			else
			{
				// Unexpected lexem
				syntax_error();
			}
			def.named_values.insert(std::make_pair(name, value));
		}
	};

	class CreateIndexParser : public Parser
	{
		IndexDef def;

	public:
		CreateIndexParser(const std::vector<Lexem>& input) : Parser(input) {}

		IndexDef parse()
		{
			accept(LexemType::CREATE);
			if (peek().type == LexemType::ORDERED)
			{
				accept(LexemType::ORDERED);
				def.is_ordered = true;
			}
			else if (peek().type == LexemType::UNORDERED)
			{
				accept(LexemType::UNORDERED);
				def.is_ordered = false;
			}
			else
			{
				// Unexpected lexem
				syntax_error();
			}
			accept(LexemType::INDEX);
			accept(LexemType::ON);
			def.name = accept(LexemType::ID).value;
			accept(LexemType::BY);
			parse_column_list();
			accept(LexemType::EOQ);

			return def;
		}

		void parse_column_list()
		{
			def.columns.push_back(accept(LexemType::ID).value);
			parse_column_list_tail();
		}

		void parse_column_list_tail()
		{
			if (peek().type == LexemType::COMMA)
			{
				accept(LexemType::COMMA);
				parse_column_list();
			}
		}
	};	

	class SelectParser : public Parser
	{
		SelectDef def;

	public:
		SelectParser(const std::vector<Lexem>& input) : Parser(input) {}

		SelectDef parse()
		{
			accept(LexemType::SELECT);
			parse_columns();			
			accept(LexemType::FROM);			
			def.name = accept(LexemType::ID).value;
			accept(LexemType::WHERE);
			def.ast = parse_or();
			accept(LexemType::EOQ);

			CondSimplifyVisitor visitor;
			def.ast = visitor.visit(def.ast);
			return def;
		}
	private:
		void parse_columns()
		{			
			def.columns.push_back(accept(LexemType::ID).value);
			while (peek().type == LexemType::COMMA)
			{
				accept(LexemType::COMMA);
				def.columns.push_back(accept(LexemType::ID).value);
			}
		}

		ASTNode* parse_or()
		{			
			ASTNode* node = parse_xor();
			while (peek().type == LexemType::OR)
			{
				const auto& lex = accept(peek().type);
				node = new InternalNode(lex, node, parse_xor());
			}
			return node;
		}

		ASTNode* parse_xor()
		{			
			ASTNode* node = parse_and();
			while (peek().type == LexemType::XOR)
			{
				const auto& lex = accept(peek().type);
				node = new InternalNode(lex, node, parse_and());
			}
			return node;
		}

		ASTNode* parse_and()
		{			
			ASTNode* node = parse_rel();
			while (peek().type == LexemType::AND)
			{
				const auto& lex = accept(peek().type);
				node = new InternalNode(lex, node, parse_rel());
			}
			return node;
		}

		ASTNode* parse_rel()
		{			
			ASTNode* node = parse_sum_expr();
			if (is_rel_op(peek()))
			{
				const auto& lex = accept(peek().type);
				return new InternalNode(lex, node, parse_sum_expr());
			}
			return node;
		}

		ASTNode* parse_sum_expr()
		{			
			ASTNode* node = parse_mul_expr();
			while (peek().type == LexemType::PLUS || peek().type == LexemType::MINUS)
			{
				const auto& lex = accept(peek().type);
				node = new InternalNode(lex, node, parse_mul_expr());
			}
			return node;
		}

		ASTNode* parse_mul_expr()
		{			
			ASTNode* node = parse_factor();
			while (peek().type == LexemType::MULT || peek().type == LexemType::DIV || peek().type == LexemType::MOD)
			{
				const auto& lex = accept(peek().type);
				node = new InternalNode(lex, node, parse_factor());
			}
			return node;
		}


		ASTNode* parse_factor()
		{			
			if (peek().type == LexemType::PLUS || peek().type == LexemType::MINUS)
			{
				// Unary operation
				const auto& lex = accept(peek().type);
				return new InternalNode(lex, parse_factor(), nullptr);
			}
			if (peek().type == LexemType::NOT)
			{
				// Unary operation
				const auto& lex = accept(peek().type);
				return new InternalNode(lex, parse_factor(), nullptr);
			}
			if (peek().type == LexemType::ID)
			{
				// Variable
				const auto& lex = accept(peek().type);
				return new LeafNode(lex);
			}
			else if (is_literal(peek()))
			{
				// Literal
				const auto& lex = accept(peek().type);
				return new LeafNode(lex);
			}
			else if (peek().type == LexemType::LPAR)
			{
				// Parenthesis
				accept(LexemType::LPAR);
				ASTNode* node = parse_or();
				accept(LexemType::RPAR);
				return node;
			}
			else
			{
				syntax_error();
			}
			return nullptr;
		}
	};
}