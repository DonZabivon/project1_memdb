#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <iterator>
#include <memory>

#include "base.h"
#include "resultrow.h"

namespace memdb
{

    class ResultSetIterator
    {
        using iterator_category = std::forward_iterator_tag;
        using value_type = ResultRow;
        using difference_type = ptrdiff_t;
        using pointer = ResultRow *;
        using reference = ResultRow &;

        friend class ResultSet;
        uint16_t row_size;
        size_t row_count;
        size_t curr_row;
        uint8_t *storage = nullptr;
        const std::unordered_map<std::string, Column> *columns;

    public:
        ResultSetIterator(const ResultSetIterator &other) : row_size(other.row_size),
                                                            row_count(other.row_count),
                                                            curr_row(other.curr_row),
                                                            storage(other.storage),
                                                            columns(other.columns)
        {
        }

        ResultSetIterator &operator++()
        {
            ++curr_row;
            return *this;
        }
        ResultSetIterator operator++(int)
        {
            ResultSetIterator tmp(*this);
            operator++();
            return tmp;
        }
        bool operator==(const ResultSetIterator &rhs) const { return curr_row == rhs.curr_row; }
        bool operator!=(const ResultSetIterator &rhs) const { return curr_row != rhs.curr_row; }
        ResultRow operator*() { return ResultRow(storage + curr_row * row_size, columns); }

    private:
        ResultSetIterator(uint16_t row_size, size_t row_count, size_t curr_row, uint8_t *storage, const std::unordered_map<std::string, Column> *columns)
            : row_size(row_size), row_count(row_count), curr_row(curr_row), storage(storage), columns(columns)
        {
        }
    };

    class ResultSet
    {
        friend class Table;
        friend class Database;

        // config
        uint16_t row_size = 0;
        size_t row_count = 0;        
        std::shared_ptr<uint8_t[]> storage;

        // columns
        std::vector<std::string> columns;
        std::unordered_map<std::string, Column> mapping;

        // result
        bool ok = true;
        std::string error = "OK";
        int64_t time_ms = 0;

    public:
        ResultSet() {}

        ~ResultSet()
        {            
        }

        ResultSetIterator begin() const { return ResultSetIterator(row_size, row_count, 0, storage.get(), &mapping); }
        ResultSetIterator end() const { return ResultSetIterator(row_size, row_count, row_count, storage.get(), &mapping); }

        size_t get_column_count() const { return columns.size(); }
        size_t get_row_count() const { return row_count; }

        std::vector<std::string> get_columns() const { return columns; }

        bool is_ok() const { return ok; }

        std::string get_error() const { return error; }

        int64_t get_time() const { return time_ms; }
    };

}
