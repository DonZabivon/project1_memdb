#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <chrono>

#include "base.h"
#include "bytes.h"
#include "value.h"
#include "column.h"
#include "condition.h"
#include "resultset.h"
#include "utils.h"
#include "ast.h"
#include "visitor.h"
#include "index.h"

namespace memdb
{
    // Database table class
    class Table
    {
        friend class Database;

        static constexpr size_t INITIAL_CAPACITY = 32;

        std::vector<Column> columns;
        uint16_t row_size = 0;
        size_t row_count = 0;
        size_t capacity = INITIAL_CAPACITY;
        std::unordered_map<std::string, size_t> mapping; // column name to column index mapping

        uint8_t *storage = nullptr; // data

        // Indices
        std::vector<OrderedIndex> ordered_indices;

        // Конструктор для создания новой таблицы
        Table(const std::vector<Column> &cols) : columns(cols)
        {
            for (size_t i = 0; i < columns.size(); ++i)
            {
                if (columns[i].is_auto && columns[i].type != Type::INT)
                {
                    throw std::runtime_error("Autoincrement is only allowed for numeric columns.");
                }
                if (columns[i].has_default && columns[i].type != columns[i].def_value.type)
                {
                    throw std::runtime_error("The default value type does not match the column type.");
                }
                columns[i].offset = row_size;
                row_size += columns[i].size;
                mapping.insert(std::make_pair(columns[i].name, i));
                
                if (columns[i].is_key)
                {
                    create_ordered_index(i);
                }
            }

            storage = new uint8_t[row_size * capacity];
        }

        // Конструктор для загрузки из файла
        Table(const std::vector<Column> &cols, size_t row_count)
            : columns(cols), row_count(row_count), capacity(row_count)
        {
            for (size_t i = 0; i < columns.size(); ++i)
            {
                columns[i].offset = row_size;
                row_size += columns[i].size;
                mapping.insert(std::make_pair(columns[i].name, i));
            }
        }

        ~Table()
        {
            delete[] storage;
        }

        // Inserts values into the table
        ResultSet insert(const std::vector<Value> &values)
        {
            ResultSet rs;
            std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

            try
            {
                std::vector<Value> v = check_inserted_values(values);
                size_t idx = row_count;
                add_row();

                uint8_t *row_ptr = storage + idx * row_size;
                for (size_t i = 0; i < columns.size(); ++i)
                {
                    size_t offset = columns[i].offset;
                    uint8_t *val_ptr = row_ptr + offset;
                    std::copy(v[i].val_ptr, v[i].val_ptr + v[i].size, val_ptr);
                }

                // update ordered indices
                for (size_t i = 0; i < ordered_indices.size(); ++i)
                {
                    OrderedIndex& ordered_index = ordered_indices[i];
                    if (columns[ordered_index.col].is_auto)
                    {
                        // just add to the end for autoicrement column
                        ordered_index.index.push_back(idx);
                    }
                    else
                    {
                        // find a position to insert using binary search
                        size_t first = upper_bound(v[ordered_index.col], ordered_index);
                        ordered_index.index.insert(ordered_index.index.begin() + first, idx);
                    }                                       
                }
            }
            catch (std::runtime_error &e)
            {
                rs.ok = false;
                rs.error = e.what();
            }

            std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
            rs.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            return rs;
        }

        // Selects all rows and all columns
        ResultSet select_all()
        {
            std::vector<std::string> cols;
            for (const auto& c : columns)
                cols.push_back(c.name);
            return select(cols, std::vector<std::pair<Condition, size_t>>());
        }

        // Selects specific columns by the given conditions
        ResultSet select(const std::vector<std::string>& cols, std::vector<std::pair<Condition, size_t>> conditions)
        {
            std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

            ResultSet rs = init_result_set(cols);
            std::vector<size_t> included_rows;
            std::unordered_set<size_t> cond_set;            

            // Check if ordered indices can be used.
            // To do this, a list of ranges found by applying 
            // binary search to the corresponding indices is created.
            std::vector<IndexRange> ranges;
            bool more = true;
            while (more)
            {
                more = false;
                for (size_t i = 0; i < ordered_indices.size(); ++i)
                {
                    const OrderedIndex &index = ordered_indices[i];
                    for (size_t j = 0; j < conditions.size(); ++j)
                    {
                        if (cond_set.count(j) > 0)
                            continue;

                        const Condition &cond = conditions[j].first;
                        if (cond.op == RelOp::NE) // TODO: "NOT EQUAL" required special processing
                            continue;

                        size_t col = conditions[j].second;
                        if (index.col == col)
                        {
                            const auto& r = select_by_index(index, cond);
                            ranges.insert(ranges.end(), r.begin(), r.end());
                            cond_set.insert(j);
                            more = true;
                            break;
                        }
                    }
                }
            }

            // This list is then reduced so that only one range 
            // remains for each index.
            if (!ranges.empty())
            {
                std::sort(ranges.begin(), ranges.end());
                std::vector<IndexRange> combined;
                IndexRange r = ranges[0];

                for (size_t i = 1; i < ranges.size(); ++i)
                {
                    if (ranges[i].index->col != r.index->col)
                    {
                        combined.push_back(r);
                        r = ranges[i];
                    }
                    else
                    {
                        if (r.begin < ranges[i].begin)
                            r.begin = ranges[i].begin;
                        if (r.end > ranges[i].end)
                            r.end = ranges[i].end;
                    }
                }
                combined.push_back(r);
                ranges.swap(combined);
                // The ranges are then sorted in order of increasing range width - 
                // the narrowest (and therefore most preferred) range is first.
                std::sort(ranges.begin(), ranges.end(), [](const IndexRange &x, const IndexRange &y)
                          { return IndexRange::compare_by_size(x, y); });
            }

            if (!ranges.empty())
            {
                // select using a range obtained by ordered index -
                // this can significantly narrow the range of rows that are checked.
                IndexRange range = ranges[0];
                for (size_t range_idx = range.begin; range_idx < range.end; ++range_idx)
                {
                    size_t row_idx = range.index->index[range_idx];

                    bool match = true;
                    for (size_t c = 0; c < conditions.size() && match; ++c)
                    {
                        size_t col_idx = conditions[c].second;
                        //if (col_idx == range.index->col)
                        //    continue;

                        const Condition &cond = conditions[c].first;
                        match = cond.match(value_at(row_idx, columns[col_idx]));
                    }

                    if (match)
                    {
                        included_rows.push_back(row_idx);
                    }
                }
                std::sort(included_rows.begin(), included_rows.end());
            }
            else
            {
                // select without using indices - check from the first to the last row.
                for (size_t row_idx = 0; row_idx < row_count; ++row_idx)
                {

                    bool match = true;
                    for (size_t c = 0; c < conditions.size() && match; ++c)
                    {
                        const Condition &cond = conditions[c].first;
                        size_t col_idx = conditions[c].second;
                        match = cond.match(value_at(row_idx, columns[col_idx]));
                    }

                    if (match)
                    {
                        included_rows.push_back(row_idx);
                    }
                }
            }

            make_resultset(included_rows, rs);
            std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
            rs.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            return rs;
        }

        // Select specific columns based on conditions 
        // given as Abstract Syntax Tree.
        ResultSet select(const std::vector<std::string>& cols, ASTNode* ast)
        {            
            std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
            ResultSet rs = init_result_set(cols);
            std::vector<size_t> included_rows;

            try
            {
                // Check the column list
                for (const auto& col_name : cols)
                {
                    if (mapping.count(col_name) == 0)
                        throw std::runtime_error("Unknown column \"" + col_name + "\" in the column list.");
                }
                // Check the condition
                SymbolVisitor visitor;
                const auto& symbols = visitor.visit(ast);
                for (const auto& item : symbols)
                {
                    if (mapping.count(item.first) == 0)
                        throw std::runtime_error("Unknown symbol \"" + item.first + "\" in the condition.");
                }
                
                // Try to convert the condition to the simple form like
                // x < 1 && y > 2 && z = 3 && ...
                if (is_cond_index_friendly(ast) && is_condition_simple(ast))
                {
                    std::vector<ASTNode*> terms = split_cond_by_and(ast);
                    // make simple cond
                    std::vector<std::pair<Condition, size_t>> conditions;
                    for (const auto& term : terms)
                    {
                        InternalNode* internal_node = dynamic_cast<InternalNode*>(term);
                        if (!internal_node)
                        {
                            LeafNode* leaf = dynamic_cast<LeafNode*>(term);
                            // assert(is_id(leaf->lexem))
                            size_t col = mapping.at(leaf->lexem.value);
                            Condition cond(Value(true), RelOp::EQ);
                            conditions.push_back(std::make_pair(cond, col));
                        }
                        else
                        {
                            LeafNode* left = dynamic_cast<LeafNode*>(internal_node->left);
                            LeafNode* right = dynamic_cast<LeafNode*>(internal_node->right);
                            if (is_id(left->lexem))
                            {
                                size_t col = mapping.at(left->lexem.value);                                
                                Condition cond(lex_to_value(right->lexem), op_to_relop(internal_node->op));
                                conditions.push_back(std::make_pair(cond, col));
                            } 
                            else
                            {
                                size_t col = mapping.at(right->lexem.value);
                                Condition cond(lex_to_value(left->lexem), op_to_relop(internal_node->op));
                                conditions.push_back(std::make_pair(cond, col));
                            }
                        }
                    }
                    return select(cols, conditions);
                }

                // Condition is not simple
                for (size_t row_idx = 0; row_idx < row_count; ++row_idx)
                {
                    // Replace symbols in the symbol table by the real values
                    std::vector<Value*> values;
                    for (auto& item : symbols)
                    {
                        size_t col = mapping.at(item.first);
                        values.push_back(new Value(value_at(row_idx, columns[col])));
                        for (auto& x : item.second)
                        {
                            x->value = values.back();
                        }
                    }

                    // Evaluate condition
                    EvalVisitor evaluator;
                    Value match = evaluator.visit(ast);
                    
                    // Check result
                    if (match.get<bool>())
                    {
                        included_rows.push_back(row_idx);
                    }

                    for (auto& val : values)
                    {
                        delete val;
                    }
                }

                make_resultset(included_rows, rs);
            }
            catch (std::runtime_error& e)
            {
                rs.ok = false;
                rs.error = e.what();
            }

            std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
            rs.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            return rs;
        }

        ResultSet init_result_set(const std::vector<std::string>& cols)
        {
            ResultSet rs;            
            rs.row_size = 0;
            for (size_t i = 0; i < cols.size(); ++i)
            {
                const auto& name = cols[i];
                size_t col_idx = mapping.at(name);
                const auto& this_column = columns[col_idx];
                Column rs_column = this_column;
                rs_column.offset = rs.row_size;
                rs.row_size += rs_column.size;
                rs.columns.push_back(name);
                rs.mapping.insert(std::make_pair(name, rs_column));
            }
            return rs;
        }

        bool is_same_order(const std::vector<std::string>& cols)
        {
            if (cols.size() != columns.size())
                return false;
            for (size_t i = 0; i < cols.size(); ++i)
            {
                if (cols[i] != columns[i].name)
                    return false;
            }
            return true;
        }

        void make_resultset(const std::vector<size_t>& included_rows, ResultSet& rs)
        {            
            rs.row_count = included_rows.size();
            rs.storage.reset(new uint8_t[rs.row_size * rs.row_count]);

            if (is_same_order(rs.columns))
            {
                // include entire row
                for (size_t rs_row_idx = 0; rs_row_idx < rs.row_count; ++rs_row_idx)
                {
                    size_t row_idx = included_rows[rs_row_idx];
                    uint8_t* row_ptr = storage + row_idx * row_size;
                    uint8_t* rs_row_ptr = rs.storage.get() + rs_row_idx * rs.row_size;                    
                    std::copy(row_ptr, row_ptr + row_size, rs_row_ptr);
                }
            }
            else
            {
                for (size_t rs_row_idx = 0; rs_row_idx < rs.row_count; ++rs_row_idx)
                {
                    size_t row_idx = included_rows[rs_row_idx];
                    uint8_t* row_ptr = storage + row_idx * row_size;
                    uint8_t* rs_row_ptr = rs.storage.get() + rs_row_idx * rs.row_size;
                    for (const auto& col_name : rs.columns)
                    {
                        const auto& col = columns[mapping.at(col_name)];
                        const auto& rs_col = rs.mapping.at(col_name);
                        uint8_t* val_ptr = row_ptr + col.offset;
                        uint8_t* rs_val_ptr = rs_row_ptr + rs_col.offset;
                        std::copy(val_ptr, val_ptr + col.size, rs_val_ptr);
                    }                    
                }
            }            
        }
        
        std::vector<IndexRange> select_by_index(const OrderedIndex &index, const Condition &cond)
        {
            if (cond.op == RelOp::EQ)
            {
                size_t first = lower_bound(cond.rhs, index);
                size_t second = upper_bound(cond.rhs, index);
                return { IndexRange(&index, first, second) };                
            }
            if (cond.op == RelOp::NE)
            {
                // TODO
                size_t first = lower_bound(cond.rhs, index);
                size_t second = upper_bound(cond.rhs, index);
                return { IndexRange(&index, 0, first) , IndexRange(&index, second, row_count) };
            }            
            if (cond.op == RelOp::LT)
            {
                size_t first = lower_bound(cond.rhs, index);
                return { IndexRange(&index, 0, first) };
            }
            if (cond.op == RelOp::GT)
            {                
                size_t first = upper_bound(cond.rhs, index);
                return { IndexRange(&index, first, row_count) };
            }
            if (cond.op == RelOp::LE)
            {                
                size_t first = upper_bound(cond.rhs, index);
                return { IndexRange(&index, 0, first) };
            }  
            if (cond.op == RelOp::GE)
            {
                size_t first = lower_bound(cond.rhs, index);
                return { IndexRange(&index, first, row_count) };
            }
            return { IndexRange(&index, row_count, row_count) };
        }

        ResultSet create_ordered_index(const std::vector<std::string> &cols)
        {
            ResultSet rs;
            std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

            try
            {
                for (const auto &col : cols)
                {
                    if (mapping.count(col) == 0)
                    {
                        throw std::runtime_error("No column named \"" + col + "\" was found.");
                    }
                    // check existence of the index
                    size_t col_idx = mapping.at(col);
                    if (has_ordered_index(col_idx))
                    {
                        throw std::runtime_error("Ordered index by \"" + col + "\" already exists.");                            
                    }
                }

                for (const auto &col : cols)
                {
                    create_ordered_index(mapping.at(col));
                }
            }
            catch (std::runtime_error &e)
            {
                rs.ok = false;
                rs.error = e.what();
            }

            std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
            rs.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            return rs;
        }

        void save_to_file(std::ostream &out) const
        {
            // Write columns info
            write_int(out, columns.size());
            for (const auto &c : columns)
            {
                c.save_to_file(out);
            }

            // Write data
            write_int(out, row_count);
            out.write((const char *)storage, row_size * row_count);

            // Write ordered indices
            write_int(out, ordered_indices.size());
            for (const auto &idx : ordered_indices)
            {
                write_int(out, idx.col);
                out.write((const char *)idx.index.data(), row_count * sizeof(size_t));
            }
        }

        static Table *load_from_file(std::istream &in)
        {
            // Read columns info
            size_t num_cols = read_int<size_t>(in);
            std::vector<Column> columns;
            columns.reserve(num_cols);
            for (size_t i = 0; i < num_cols; ++i)
            {
                Column column = Column::load_from_file(in);
                columns.push_back(column);
            }

            // Read data
            size_t row_count = read_int<size_t>(in);
            Table *table = new Table(columns, row_count);
            table->storage = new uint8_t[table->row_size * table->capacity];
            in.read((char *)table->storage, table->row_size * table->row_count);

            // Read ordered indices
            size_t num_idx = read_int<size_t>(in);
            table->ordered_indices.reserve(num_idx);
            for (size_t i = 0; i < num_idx; ++i)
            {
                size_t col = read_int<size_t>(in);
                OrderedIndex idx(col);
                size_t *data = new size_t[table->row_count];
                in.read((char *)data, table->row_count * sizeof(size_t));
                idx.index = std::vector(data, data + table->row_count);
                table->ordered_indices.push_back(idx);
            }

            return table;
        }

        Value value_at(size_t row, const Column &column) const
        {
            uint8_t* val_ptr = storage + row * row_size + column.offset;
            return Value(column.type, val_ptr, column.size);
        }

        void add_row()
        {
            if (row_count == capacity)
            {
                capacity *= 2;
                uint8_t *new_storage = new uint8_t[row_size * capacity];
                std::copy(storage, storage + row_size * (capacity / 2), new_storage);
                delete[] storage;
                storage = new_storage;
            }
            row_count++;
        }

        std::vector<Value> check_inserted_values(std::vector<Value> values)
        {

            if (values.size() != columns.size())
            {
                throw std::runtime_error("Columns mismatch");
            }

            for (size_t i = 0; i < columns.size(); ++i)
            {
                if (columns[i].type != values[i].type &&
                    !(columns[i].has_default && values[i].type == Type::NONE) &&
                    !(columns[i].is_auto && values[i].type == Type::NONE))
                {
                    throw std::runtime_error("Type mismatch");
                }
                if (columns[i].type == Type::STRING && values[i].size > columns[i].size)
                {
                    throw std::runtime_error("Size too large");
                }
                if (columns[i].type == Type::BYTES && values[i].size != columns[i].size)
                {
                    throw std::runtime_error("Size too large");
                }
                if (columns[i].is_auto)
                {
                    values[i] = Value(columns[i].autoincrement_value++);
                }
                if (values[i].type == Type::NONE)
                {
                    values[i] = columns[i].def_value; // replace with default
                }
                if (!columns[i].is_auto && (columns[i].is_unique || columns[i].is_key))
                {
                    // check uniqueness
                    if (!check_unique_value(values[i], i))
                        throw std::runtime_error("Value is not unique");
                }
            }
            return values;
        }

        bool has_ordered_index(size_t col_idx)
        {
            for (const auto& index : ordered_indices)
            {
                if (index.col == col_idx)
                    return true;
            }
            return false;
        }

        OrderedIndex* get_ordered_index(size_t col_idx)
        {
            for (auto& index : ordered_indices)
            {
                if (index.col == col_idx)
                    return &index;
            }
            return nullptr;
        }

        bool check_unique_value(const Value& val, size_t col_idx)
        {            
            OrderedIndex* index_ptr = get_ordered_index(col_idx);
            if (index_ptr)
            {
                // if ordered index exists
                size_t first = binary_search(val, *index_ptr);
                return first == row_count;
            }
            else
            {
                // no index
                for (size_t row_idx = 0; row_idx < row_count; ++row_idx)
                {
                    if (value_at(row_idx, columns[col_idx]) == val)
                        return false;
                }
                return true;
            }            
        }

        void create_ordered_index(size_t col)
        {
            // assert(has_ordered_index(col) == false)

            ordered_indices.push_back(OrderedIndex(col));
            std::vector<size_t> &index = ordered_indices.back().index;
            index.reserve(row_count);
            for (size_t i = 0; i < row_count; ++i)
            {
                index.push_back(i);
            }
            update_ordered_index(ordered_indices.back());
        } 

        void update_ordered_index(OrderedIndex& ordered_index)
        {            
            std::vector<size_t>& index = ordered_index.index;
            const Column& c = columns[ordered_index.col];
            uint8_t* st = storage;
            uint16_t rsz = row_size;
            std::sort(index.begin(), index.end(), [&c, st, rsz](size_t a, size_t b) {
                Value val1 = Value(c.type, st + a * rsz + c.offset, c.size);
                Value val2 = Value(c.type, st + b * rsz + c.offset, c.size);
                return val1 < val2;
            });
        }

        size_t lower_bound(const Value &val, const OrderedIndex &index) const
        {
            const Column &column = columns[index.col];
            size_t first = 0;
            size_t last = index.index.size();
            int64_t count = (int64_t)last - first;
            while (count > 0)
            {
                int64_t step = count / 2;
                size_t mid = first + step;
                Value it = value_at(index.index[mid], column);
                if (it < val)
                {
                    first = mid + 1;
                    count -= step + 1;
                }
                else
                {
                    count = step;
                }
            }
            return first;
        }

        size_t upper_bound(const Value &val, const OrderedIndex &index) const
        {
            const Column &column = columns[index.col];
            size_t first = 0;
            size_t last = index.index.size();
            int64_t count = (int64_t)last - first;
            while (count > 0)
            {
                int64_t step = count / 2;
                size_t mid = first + step;
                Value it = value_at(index.index[mid], column);
                if (!(val < it))
                {
                    first = mid + 1;
                    count -= step + 1;
                }
                else
                {
                    count = step;
                }
            }
            return first;
        }

        size_t binary_search(const Value &val, const OrderedIndex &index) const
        {
            const Column &column = columns[index.col];
            size_t first = lower_bound(val, index);
            if (first != index.index.size())
            {
                if (!(val < value_at(index.index[first], column)))
                    return first;
            }
            return index.index.size();
        }
    };

}
