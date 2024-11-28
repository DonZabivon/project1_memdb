#pragma once

#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <iostream>

#include "base.h"
#include "table.h"
#include "lexer.h"
#include "parser.h"
#include "utils.h"

namespace memdb
{

	class Database
	{
		std::map<std::string, Table *> tables;

	public:
		Database() {}

		~Database()
		{
			clear();
		}

		void clear()
		{
			for (auto &p : tables)
			{
				delete p.second;
			}
			tables.clear();
		}

		ResultSet create_table(const std::string &name, const std::vector<Column> &columns)
		{
			try
			{
				if (!check_column_names(columns))
				{
					throw std::runtime_error("The column definition contains duplicate names.");
				}
				if (find(name) != nullptr)
				{
					throw std::runtime_error("A table with the given name already exists.");
				}
				Table *table = new Table(columns);
				tables.insert(std::make_pair(name, table));
				return ResultSet();
			}
			catch (std::runtime_error &e)
			{
				return error_result(e.what());
			}
		}

		ResultSet insert(const std::string &name, const std::vector<Value> &values)
		{
			try
			{
				Table *table = get(name);
				return table->insert(values);
			}
			catch (std::runtime_error &e)
			{
				return error_result(e.what());
			}
		}

		ResultSet select_all(const std::string &name)
		{
			return select(name, std::vector<std::pair<Condition, size_t>>());
		}

		ResultSet select(const std::string &name, std::vector<std::pair<Condition, size_t>> conditions)
		{
			try
			{
				Table *table = get(name);
				return table->select(conditions);
			}
			catch (std::runtime_error &e)
			{
				return error_result(e.what());
			}
		}

		ResultSet select(const std::string& name, const std::vector<std::string>& columns, ASTNode* ast)
		{
			try
			{
				Table* table = get(name);
				return table->select(columns, ast);
			}
			catch (std::runtime_error& e)
			{
				return error_result(e.what());
			}
		}

		ResultSet create_ordered_index(const std::string &table_name, const std::vector<std::string> &columns)
		{
			try
			{
				Table *table = get(table_name);
				return table->create_ordered_index(columns);
			}
			catch (std::runtime_error &e)
			{
				return error_result(e.what());
			}
		}

		ResultSet execute(const std::string &query)
		{
			try
			{
				Lexer lexer(query);
				const auto &lexems = lexer.tokenize();
				if (lexems.empty())
				{
					throw std::runtime_error("Empty query.");
				}
				if (lexems[0].type == LexemType::CREATE)
				{
					if (lexems.size() < 2)
					{
						throw std::runtime_error("Query too short.");
					}

					if (lexems[1].type == LexemType::TABLE)
					{
						CreateTableParser parser(lexems);
						CreateTableDef def = parser.parse();
						return create_table(def.name, def.columns);
					}
					else
					{
						CreateIndexParser parser(lexems);
						IndexDef def = parser.parse();
						if (def.is_ordered)
						{
							return create_ordered_index(def.name, def.columns);
						}
					}
				}
				else if (lexems[0].type == LexemType::INSERT)
				{
					InsertParser parser(lexems);
					InsertDef def = parser.parse();
					if (!def.using_named_values)
					{
						return insert(def.name, def.values);
					}
					// Prepare values
					std::vector<Value> values;
					Table *table = get(def.name);
					for (const auto &column : table->columns)
					{
						if (def.named_values.count(column.name) > 0)
						{
							values.push_back(def.named_values.at(column.name));
						}
						else
						{
							values.push_back(Value()); // to use default
						}
					}
					return insert(def.name, values);
				}
				else if (lexems[0].type == LexemType::SELECT)
				{
					SelectParser parser(lexems);
					SelectDef def = parser.parse();

					return select(def.name, def.columns, def.ast);
				}
				throw std::runtime_error("Not implemented yet");
			}
			catch (std::runtime_error &e)
			{
				return error_result(e.what());
			}
		}

		void save_to_file(std::ostream &out) const
		{			
			write_int(out, tables.size());			
			for (const auto &p : tables)
			{
				const auto &name = p.first;
				Table *table = p.second;
				write_string(out, name);
				table->save_to_file(out);
			}
		}

		void load_from_file(std::istream &in)
		{
			clear();
			size_t num_tables = read_int<size_t>(in);
			for (size_t i = 0; i < num_tables; ++i)
			{
				std::string name = read_string(in);
				Table *table = Table::load_from_file(in);
				tables.insert(std::make_pair(name, table));
			}
		}

		void info(std::ostream &out)
		{
			out << "Database info:" << std::endl;
			size_t i = 1;
			for (const auto &p : tables)
			{
				const auto &name = p.first;
				Table *table = p.second;
				out << i++ << ": " << name << " (" << table->columns.size() << " columns, " << table->row_count << " rows)" << std::endl;
			}
			out << std::endl;
		}

	private:
		bool check_column_names(const std::vector<Column> &columns)
		{
			std::set<std::string> names;
			for (const auto &column : columns)
				names.insert(column.name);
			return columns.size() == names.size();
		}

		Table *find(const std::string &name)
		{
			if (tables.count(name) > 0)
				return tables.at(name);
			return nullptr;
		}

		Table *get(const std::string &name)
		{
			if (tables.count(name) > 0)
				return tables.at(name);
			throw std::runtime_error("No table with the given name was found.");
		}

		ResultSet error_result(const std::string &msg)
		{
			ResultSet rs;
			rs.ok = false;
			rs.error = msg;
			return rs;
		}
	};

}
