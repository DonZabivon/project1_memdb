#pragma once

#include <vector>

#include "lexem.h"

namespace memdb
{
	struct ASTNode
	{
		virtual ~ASTNode() {}
	};

	struct InternalNode : public ASTNode
	{
		Op op;
		ASTNode* left;
		ASTNode* right;

		InternalNode(Op op, ASTNode* left, ASTNode* right) : ASTNode(), op(op), left(left), right(right) {}

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

	inline bool is_cond_index_friendly(ASTNode* root)
	{
		InternalNode* internal_node = dynamic_cast<InternalNode*>(root);
		return
			internal_node == nullptr ||
			(internal_node->op != Op::OR && internal_node->op != Op::XOR);
	}

	inline std::vector<ASTNode*> split_cond_by_and(ASTNode* root)
	{
		// assert(is_cond_index_friendly(root))

		std::vector<ASTNode*> terms;
		
		InternalNode* internal_node = dynamic_cast<InternalNode*>(root);
		while (internal_node && internal_node->op == Op::AND)
		{
			
			ASTNode* left = internal_node->left;
			ASTNode* right = internal_node->right;
			internal_node->left = nullptr;
			internal_node->right = nullptr;
			terms.push_back(right);
			root = left;
			internal_node = dynamic_cast<InternalNode*>(root);
		}
		terms.push_back(root);

		return terms;
	}



	inline bool is_expr_simple(ASTNode* root)
	{
		InternalNode* internal_node = dynamic_cast<InternalNode*>(root);
		if (!internal_node)
		{
			return true;
		}
		if (is_rel_op(internal_node->op))
		{
			LeafNode* left = dynamic_cast<LeafNode*>(internal_node->left);
			LeafNode* right = dynamic_cast<LeafNode*>(internal_node->right);
			if (left && right)
			{
				return
					(is_id(left->lexem) && is_literal(right->lexem)) ||
					(is_id(right->lexem) && is_literal(left->lexem));
			}
		}
		return false;
	}

	inline bool is_condition_simple(ASTNode* root)
	{
		// assert(is_cond_index_friendly(root))		

		InternalNode* internal_node = dynamic_cast<InternalNode*>(root);
		while (internal_node && internal_node->op == Op::AND)
		{								
			if (!is_expr_simple(internal_node->right))
				return false;
			root = internal_node->left;
			internal_node = dynamic_cast<InternalNode*>(root);
		}
		return is_expr_simple(root);
	}

	inline bool is_condition_simple(const std::vector<ASTNode*>& terms)
	{
		for (const auto& term : terms)
		{
			if (!is_expr_simple(term))
				return false;
			return true;
		}
	}
}
