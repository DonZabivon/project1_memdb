#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>

#include "base.h"
#include "value.h"

namespace memdb
{

    class ResultRow
    {
        friend class ResultSetIterator;
        uint8_t *row_ptr;
        const std::unordered_map<std::string, Column> *columns = nullptr;
        ResultRow(uint8_t *row_ptr, const std::unordered_map<std::string, Column> *columns) : row_ptr(row_ptr), columns(columns) {}

    public:
        template <typename T>
        T get(const std::string &name) const
        {
            throw std::runtime_error("Invalid type");
        }

        template <typename T>
        const T get(size_t i) const
        {
            throw std::runtime_error("Not implemented");
        }
    };

    template <>
    int32_t ResultRow::get<int32_t>(const std::string &name) const
    {
        const Column &column = columns->at(name);
        if (column.type != Type::INT)
        {
            throw std::runtime_error("Invalid type");
        }
        size_t offset = column.offset;
        uint8_t *val_ptr = row_ptr + offset;
        return *((int32_t *)val_ptr);
    }

    template <>
    bool ResultRow::get<bool>(const std::string &name) const
    {
        const Column &column = columns->at(name);
        if (column.type != Type::BOOL)
        {
            throw std::runtime_error("Invalid type");
        }
        size_t offset = column.offset;
        uint8_t *val_ptr = row_ptr + offset;
        return *((bool *)val_ptr);
    }

    template <>
    std::string ResultRow::get<std::string>(const std::string &name) const
    {
        const Column &column = columns->at(name);
        if (column.type != Type::STRING)
        {
            throw std::runtime_error("Invalid type");
        }
        size_t offset = column.offset;
        uint8_t *val_ptr = row_ptr + offset;
        return std::string((const char *)val_ptr);
    }

    template <>
    Bytes ResultRow::get<Bytes>(const std::string &name) const
    {
        const Column &column = columns->at(name);
        if (column.type != Type::BYTES)
        {
            throw std::runtime_error("Invalid type");
        }
        size_t offset = column.offset;
        uint8_t *val_ptr = row_ptr + offset;
        return Bytes(val_ptr, val_ptr + column.size);
    }

}
