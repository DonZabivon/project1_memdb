#pragma once

#include <stdexcept>
#include <string>
#include <iostream>

#include "base.h"
#include "value.h"
#include "utils.h"

namespace memdb
{

    struct Column
    {
        Type type = Type::NONE;
        std::string name;
        uint16_t size = 0;
        uint16_t offset = 0;
        bool is_unique = false;
        bool is_auto = false;
        bool is_key = false;
        bool has_default = false;
        int32_t autoincrement_value = 1;
        Value def_value;

        Column() {}

        Column(Type type, const std::string &name, uint16_t size, const Value &def_value)
            : type(type), name(name), size(size), has_default(true), def_value(def_value) {}

        Column(Type type, const std::string &name, uint16_t size) : type(type), name(name), size(size) {}

        Column(Type type, const std::string &name) : type(type), name(name)
        {
            if (type == Type::INT)
            {
                size = sizeof(int32_t);
            }
            else if (type == Type::BOOL)
            {
                size = sizeof(bool);
            }
            else
            {
                throw new std::runtime_error("Missing size");
            }
        }

        void save_to_file(std::ostream &out) const
        {
            write_int(out, (int)type);
            write_string(out, name);
            write_int(out, size);
            write_int(out, offset);
            write_int(out, is_unique);
            write_int(out, is_auto);
            write_int(out, is_key);
            write_int(out, has_default);
            if (is_auto)
            {
                write_int(out, autoincrement_value);
            }
            if (has_default)
            {
                if (type == Type::INT)
                    write_int(out, def_value.get<int32_t>());
                else if (type == Type::BOOL)
                    write_int(out, def_value.get<bool>());
                else if (type == Type::STRING)
                    write_string(out, def_value.get<std::string>());
                else if (type == Type::BYTES)
                    write_bytes(out, def_value.get<Bytes>());
            }
        }

        static Column load_from_file(std::istream &in)
        {
            Column column;

            column.type = (Type)read_int<int>(in);
            column.name = read_string(in);
            column.size = read_int<uint16_t>(in);
            column.offset = read_int<uint16_t>(in);
            column.is_unique = read_int<bool>(in);
            column.is_auto = read_int<bool>(in);
            column.is_key = read_int<bool>(in);
            column.has_default = read_int<bool>(in);
            if (column.is_auto)
            {
                column.autoincrement_value = read_int<int32_t>(in);
            }
            if (column.has_default)
            {
                if (column.type == Type::INT)
                    column.def_value = Value(read_int<int32_t>(in));
                else if (column.type == Type::BOOL)
                    column.def_value = Value(read_int<bool>(in));
                else if (column.type == Type::STRING)
                    column.def_value = Value(read_string(in));
                else if (column.type == Type::BYTES)
                    column.def_value = Value(read_bytes(in));
            }

            return column;
        }
    };

}
