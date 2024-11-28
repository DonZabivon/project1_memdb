#pragma once

#include "lexem.h"

namespace memdb
{
	struct ASTNode
	{
		virtual ~ASTNode() {}
	};

	struct InternalNode : public ASTNode
	{
		Lexem lexem;
		ASTNode* left;
		ASTNode* right;

		InternalNode(const Lexem& lexem, ASTNode* left, ASTNode* right) : ASTNode(), lexem(lexem), left(left), right(right) {}

		~InternalNode()
		{
			delete left;
			delete right;
		}
	};

	struct LeafNode : public ASTNode
	{
		Lexem lexem;
		Value* value = nullptr;

		LeafNode(const Lexem& lexem) : ASTNode(), lexem(lexem) {}
	};
}
