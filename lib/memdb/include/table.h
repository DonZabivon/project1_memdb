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

namespace memdb
{

    struct OrderedIndex
    {
        size_t col;
        std::vector<size_t> index;

        OrderedIndex(size_t col) : col(col) {}

        friend bool operator<(const OrderedIndex &x, const OrderedIndex &y)
        {
            return x.col < y.col;
        }
    };

    struct IndexRange
    {
        const OrderedIndex *index;
        size_t begin;
        size_t end;

        IndexRange(const OrderedIndex *index, size_t begin, size_t end) : index(index), begin(begin), end(end) {}

        size_t size() const { return end - begin; }

        static bool compare_by_size(const IndexRange &x, const IndexRange &y)
        {
            return x.size() < y.size();
        }

        friend bool operator<(const IndexRange &x, const IndexRange &y)
        {
            if ((*x.index) < (*y.index))
                return true;
            if ((*y.index) < (*x.index))
                return false;
            if (x.begin < y.begin)
                return true;
            if (y.begin < x.begin)
                return false;
            return x.end < y.end;
        }
    };

    class Table
    {
        friend class Database;

        static constexpr size_t INITIAL_CAPACITY = 32;

        std::vector<Column> columns;
        uint16_t row_size = 0;
        size_t row_count = 0;
        size_t capacity = INITIAL_CAPACITY;
        std::unordered_map<std::string, size_t> name_to_col;

        uint8_t *storage = nullptr;

        // Indices
        std::vector<OrderedIndex> ordered_indices;

    public:
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
                name_to_col.insert(std::make_pair(columns[i].name, i));
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
                name_to_col.insert(std::make_pair(columns[i].name, i));
            }
        }

        ~Table()
        {
            delete[] storage;
        }

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

        ResultSet select_all()
        {
            return select(std::vector<std::pair<Condition, size_t>>());
        }

        ResultSet select(std::vector<std::pair<Condition, size_t>> conditions)
        {
            std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

            ResultSet rs = init_result_set();
            std::vector<size_t> included_rows;
            std::unordered_set<size_t> cond_set;

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
                        size_t col = conditions[j].second;
                        if (index.col == col)
                        {
                            auto r = select_by_index(index, cond);
                            ranges.push_back(r);
                            cond_set.insert(j);
                            more = true;
                            break;
                        }
                    }
                }
            }

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
                std::sort(ranges.begin(), ranges.end(), [](const IndexRange &x, const IndexRange &y)
                          { return IndexRange::compare_by_size(x, y); });
            }

            if (!ranges.empty())
            {
                IndexRange range = ranges[0];
                for (size_t range_idx = range.begin; range_idx < range.end; ++range_idx)
                {
                    size_t row_idx = range.index->index[range_idx];

                    bool match = true;
                    for (size_t c = 0; c < conditions.size() && match; ++c)
                    {
                        size_t col_idx = conditions[c].second;
                        if (col_idx == range.index->col)
                            continue;

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

        ResultSet select(const std::vector<std::string>& cols, ASTNode* ast)
        {            
            std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
            ResultSet rs = init_result_set();
            std::vector<size_t> included_rows;

            try
            {
                for (const auto& col_name : cols)
                {
                    if (name_to_col.count(col_name) == 0)
                        throw std::runtime_error("Unknown column \"" + col_name + "\" in the column list.");
                }
                SymbolVisitor visitor;
                const auto& symbols = visitor.visit(ast);
                for (const auto& item : symbols)
                {
                    if (name_to_col.count(item.first) == 0)
                        throw std::runtime_error("Unknown symbol \"" + item.first + "\" in the condition.");
                }

                for (size_t row_idx = 0; row_idx < row_count; ++row_idx)
                {
                    std::vector<Value*> values;
                    for (auto& item : symbols)
                    {
                        size_t col = name_to_col.at(item.first);
                        values.push_back(new Value(value_at(row_idx, columns[col])));
                        for (auto& x : item.second)
                        {
                            x->value = values.back();
                        }
                    }

                    EvalVisitor evaluator;
                    Value match = evaluator.visit(ast);
                    
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
        
        IndexRange select_by_index(const OrderedIndex &index, const Condition &cond)
        {
            if (cond.op == CondOp::EQ)
            {
                size_t first = binary_search(cond.rhs, index);
                if (first != row_count)
                    return IndexRange(&index, first, first + 1);
                else
                    return IndexRange(&index, row_count, row_count);
            }
            if (cond.op == CondOp::NE)
            {
                // TODO
                return IndexRange(&index, row_count, row_count);
            }
            if (cond.op == CondOp::GE)
            {
                size_t first = lower_bound(cond.rhs, index);
                return IndexRange(&index, first, row_count);
            }
            if (cond.op == CondOp::GT)
            {
                size_t first = lower_bound(cond.rhs, index);
                while (first < row_count && value_at(index.index[first], columns[index.col]) == cond.rhs)
                {
                    first++;
                }
                return IndexRange(&index, first, row_count);
            }
            if (cond.op == CondOp::LE)
            {
                size_t first = lower_bound(cond.rhs, index);
                while (first < row_count && value_at(index.index[first], columns[index.col]) == cond.rhs)
                {
                    first++;
                }
                return IndexRange(&index, 0, first);
            }
            if (cond.op == CondOp::LT)
            {
                size_t first = upper_bound(cond.rhs, index);
                return IndexRange(&index, 0, first);
            }
            return IndexRange(&index, row_count, row_count);
        }

        ResultSet create_ordered_index(const std::vector<std::string> &cols)
        {
            ResultSet rs;
            std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

            try
            {
                for (const auto &col : cols)
                {
                    if (name_to_col.count(col) == 0)
                    {
                        throw std::runtime_error("No column with the given name was found.");
                    }
                }

                for (const auto &col : cols)
                {
                    create_ordered_index(name_to_col.at(col));
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

    private:
        Value value_at(size_t row, const Column &column) const
        {
            return Value(column.type, storage + row * row_size + column.offset, column.size);
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
            }
            return values;
        }

        void create_ordered_index(size_t col)
        {
            ordered_indices.push_back(OrderedIndex(col));
            std::vector<size_t> &index = ordered_indices.back().index;

            index.reserve(row_count);
            for (size_t i = 0; i < row_count; ++i)
            {
                index.push_back(i);
            }

            const Column &c = columns[col];
            uint8_t *st = storage;
            uint16_t rsz = row_size;
            std::sort(index.begin(), index.end(), [&c, st, rsz](size_t a, size_t b)
                      {
                Value val1 = Value(c.type, st + a * rsz + c.offset, c.size);
                Value val2 = Value(c.type, st + b * rsz + c.offset, c.size);
                return val1 < val2; });
        }

        ResultSet init_result_set()
        {
            ResultSet rs;
            for (size_t i = 0; i < columns.size(); ++i)
            {
                const auto &column = columns[i];
                rs.columns.push_back(column.name);
                rs.mapping.insert(std::make_pair(column.name, column));
            }
            return rs;
        }

        void make_resultset(const std::vector<size_t> &included_rows, ResultSet &rs)
        {
            rs.row_size = row_size;
            rs.row_count = included_rows.size();
            rs.storage.reset(new uint8_t[rs.row_size * rs.row_count]);

            for (size_t i = 0; i < rs.row_count; ++i)
            {
                size_t row_idx = included_rows[i];
                uint8_t *row_ptr = storage + row_idx * row_size;
                uint8_t *rs_row_ptr = rs.storage.get() + i * rs.row_size;
                std::copy(row_ptr, row_ptr + row_size, rs_row_ptr);
            }
        }

        size_t lower_bound(const Value &val, const OrderedIndex &index) const
        {
            const Column &column = columns[index.col];
            size_t first = 0;
            size_t last = row_count;
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
            size_t last = row_count;
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
            if (first != row_count)
            {
                if (!(val < value_at(first, column)))
                    return first;
            }
            return row_count;
        }
    };

}
