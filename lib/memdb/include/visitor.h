#pragma once

#include <stdexcept>
#include <string>
#include <set>
#include <map>
#include <vector>

#include "lexem.h"
#include "ast.h"
#include "value.h"
#include "utils.h"

namespace memdb
{
	class CondSimplifyVisitor
	{
	public:
		CondSimplifyVisitor() {}

		ASTNode* visit(ASTNode* root)
		{
			InternalNode* op_node;
			if (op_node = dynamic_cast<InternalNode*>(root))
			{
				op_node->left = visit(op_node->left);
				op_node->right = visit(op_node->right);
				LeafNode* leaf1, * leaf2;
				if ((leaf1 = dynamic_cast<LeafNode*>(op_node->left)) && (leaf2 = dynamic_cast<LeafNode*>(op_node->right)))
				{
					if (is_literal(leaf1->lexem) && is_literal(leaf2->lexem))
					{
						Value val1 = lex_to_value(leaf1->lexem);
						Value val2 = lex_to_value(leaf2->lexem);
						if (val1.type != val2.type)
							throw std::runtime_error("Mismatch of operand types in expression");

						Type value_type = val1.type;
						Op op_type = op_node->op;

						if (is_math_op(op_type))
						{
							if (value_type == Type::INT)
							{
								int32_t result = 0;
								int32_t v1 = val1.get<int32_t>();
								int32_t v2 = val2.get<int32_t>();

								if (op_type == Op::PLS)
								{
									result = v1 + v2;
								}
								if (op_type == Op::MNS)
								{
									result = v1 - v2;
								}
								if (op_type == Op::MUL)
								{
									result = v1 * v2;
								}
								if (op_type == Op::DIV)
								{
									result = v1 / v2;
								}
								if (op_type == Op::MOD)
								{
									result = v1 % v2;
								}
								Lexem lexem;
								lexem.type = LexemType::INT_LIT;
								lexem.value = std::to_string(result);
								delete root;
								return new LeafNode(lexem);
							}
							else if (value_type == Type::STRING && op_type == Op::PLS)
							{
								std::string v1 = val1.get<std::string>();
								std::string v2 = val2.get<std::string>();
								std::string result = v1 + v2;
								Lexem lexem;
								lexem.type = LexemType::STR_LIT;
								lexem.value = result;
								delete root;
								return new LeafNode(lexem);
							}
							else
							{
								throw std::runtime_error("Mismatch between operation and operands in expression");
							}
						}
						else if (is_rel_op(op_type))
						{
							bool result = false;

							if (op_type == Op::EQ)
							{
								result = val1 == val2;
							}
							if (op_type == Op::NE)
							{
								result = val1 != val2;
							}
							if (op_type == Op::LT)
							{
								result = val1 < val2;
							}
							if (op_type == Op::GT)
							{
								result = val1 > val2;
							}
							if (op_type == Op::LE)
							{
								result = val1 <= val2;
							}
							if (op_type == Op::GE)
							{
								result = val1 >= val2;
							}

							Lexem lexem;
							lexem.type = LexemType::BOOL_LIT;
							lexem.value = result ? "true" : "false";
							delete root;
							return new LeafNode(lexem);
						}
						else if (is_logic_op(op_type))
						{
							if (value_type == Type::BOOL)
							{
								bool result = 0;
								bool v1 = val1.get<bool>();
								bool v2 = val2.get<bool>();

								if (op_type == Op::AND)
								{
									result = v1 && v2;
								}
								if (op_type == Op::OR)
								{
									result = v1 || v2;
								}
								if (op_type == Op::XOR)
								{
									result = v1 != v2;
								}
								Lexem lexem;
								lexem.type = LexemType::BOOL_LIT;
								lexem.value = result ? "true" : "false";
								delete root;
								return new LeafNode(lexem);
							}
							else
							{
								throw std::runtime_error("Logical operations are only allowed on logical values");
							}
						}
					}
				}
				if ((leaf1 = dynamic_cast<LeafNode*>(op_node->left)) && op_node->right == nullptr)
				{
					// Unary op
					if (is_literal(leaf1->lexem))
					{
						Value val1 = lex_to_value(leaf1->lexem);
						Type value_type = val1.type;
						Op op_type = op_node->op;
						if (value_type == Type::INT && (op_type == Op::PLS || op_type == Op::MNS))
						{
							int32_t result = val1.get<int32_t>();
							if (op_type == Op::MNS)
							{
								result = -result;
							}
							Lexem lexem;
							lexem.type = LexemType::INT_LIT;
							lexem.value = std::to_string(result);
							delete root;
							return new LeafNode(lexem);
						}
						else if (value_type == Type::BOOL && op_type == Op::NOT)
						{
							bool result = !val1.get<bool>();
							Lexem lexem;
							lexem.type = LexemType::BOOL_LIT;
							lexem.value = result ? "true" : "false";
							delete root;
							return new LeafNode(lexem);
						}
						else
						{
							throw std::runtime_error("Mismatch between unary operation and operand in expression");
						}
					}
				}
				return op_node;
			}
			else if (dynamic_cast<LeafNode*>(root))
			{
				// do nothing
			}
			return root;
		}
	};

	typedef std::map<std::string, std::vector<LeafNode*>> SymbolTable;

	class SymbolVisitor
	{
	public:
		SymbolVisitor() {}

		SymbolTable visit(ASTNode* root)
		{
			SymbolTable symbols;
			visit(root, symbols);
			return symbols;
		}
	private:
		void visit(ASTNode* root, SymbolTable& symbols)
		{
			InternalNode* node;
			LeafNode* leaf;
			if (node = dynamic_cast<InternalNode*>(root))
			{
				visit(node->left, symbols);
				visit(node->right, symbols);
			}
			else if (leaf = dynamic_cast<LeafNode*>(root))
			{
				if (leaf->lexem.type == LexemType::ID)
				{
					if (symbols.count(leaf->lexem.value) == 0)
					{
						symbols.insert(std::make_pair(leaf->lexem.value, std::vector<LeafNode* >()));
					}
					symbols.at(leaf->lexem.value).push_back(leaf);
				}				
			}			
		}
	};

	class EvalVisitor
	{
	public:
		EvalVisitor() {}

		Value visit(ASTNode* root)
		{
			InternalNode* op_node;
			LeafNode* leaf_node;
			if (op_node = dynamic_cast<InternalNode*>(root))
			{
				Op op_type = op_node->op;
				Value val1 = visit(op_node->left);
				Value val2 = visit(op_node->right);
				if (val2.type != Type::NONE)
				{					
					switch (op_type)
					{
					case Op::PLS:
						return val1 + val2;
					case Op::MNS:
						return val1 - val2;
					case Op::MUL:
						return val1 * val2;
					case Op::DIV:
						return val1 / val2;
					case Op::MOD:
						return val1 % val2;
					case Op::EQ:
						return val1 == val2;
					case Op::NE:
						return val1 != val2;
					case Op::LT:
						return val1 < val2;
					case Op::GT:
						return val1 > val2;
					case Op::LE:
						return val1 <= val2;
					case Op::GE:
						return val1 >= val2;
					case Op::AND:
						return val1 & val2;
					case Op::OR:
						return val1 | val2;
					case Op::XOR:
						return val1 ^ val2;
					}
					throw std::runtime_error("Unreachable");
				}
				else
				{
					// Unary op
					switch (op_type)
					{
					case Op::PLS:
						return val1;
					case Op::MNS:
						return -val1;
					case Op::NOT:
						return ~val1;					
					}
					throw std::runtime_error("Unreachable");
				}				
			}
			else if (leaf_node = dynamic_cast<LeafNode*>(root))
			{
				if (leaf_node->lexem.type == LexemType::ID)
				{
					return *leaf_node->value;
				}
				else
				{
					return lex_to_value(leaf_node->lexem);
				}
			}
			return Value(); // NULL value
		}
	};
}