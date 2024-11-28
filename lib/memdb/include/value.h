#pragma once

#include <cstring>
#include <stdexcept>
#include <string>
#include <algorithm>

#include "base.h"
#include "bytes.h"

namespace memdb
{

    struct Value
    {
        Type type;
        uint16_t size;
        uint8_t *val_ptr;
        bool owner = true;

        Value() : type(Type::NONE), size(0), val_ptr(nullptr), owner(true) {}

        Value(Type type, uint8_t *val_ptr, uint16_t size) : type(type), size(size), val_ptr(val_ptr), owner(false) {}

        Value(Type type, uint8_t *val_ptr) : type(type), val_ptr(val_ptr), owner(false)
        {
            if (type == Type::INT)
            {
                size = sizeof(int32_t);
            }
            else if (type == Type::BOOL)
            {
                size = sizeof(bool);
            }
            else if (type == Type::STRING)
            {
                size = (uint16_t)strlen((const char *)val_ptr) + 1;
            }
            else
            {
                throw new std::runtime_error("Missing size");
            }
        }

        Value(int32_t val) : type(Type::INT), size(sizeof(int32_t)), val_ptr(new uint8_t[size])
        {
            *((int32_t *)val_ptr) = val;
        }

        Value(bool val) : type(Type::BOOL), size(sizeof(bool)), val_ptr(new uint8_t[size])
        {
            *((bool *)val_ptr) = val;
        }

        Value(const std::string &val) : type(Type::STRING), size((uint16_t)val.size() + 1), val_ptr(new uint8_t[size])
        {
            std::copy(val.c_str(), val.c_str() + size, val_ptr);
        }

        Value(const Bytes &val) : type(Type::BYTES), size((uint16_t)val.size()), val_ptr(new uint8_t[size])
        {
            std::copy(val.data(), val.data() + size, val_ptr);
        }

        Value(const char *val) : Value(std::string(val))
        {
        }

        Value(const Value &other) : type(other.type), size(other.size), owner(other.owner)
        {
            if (owner)
            {
                val_ptr = new uint8_t[size];
                std::copy(other.val_ptr, other.val_ptr + size, val_ptr);
            }
            else
            {
                val_ptr = other.val_ptr;
            }
        }

        Value &operator=(const Value &other)
        {
            if (&other != this)
            {
                if (owner != other.owner)
                {
                    throw std::runtime_error("Ownership strategy of values does not match");
                }

                type = other.type;
                size = other.size;

                if (owner)
                {
                    delete[] val_ptr;
                    val_ptr = new uint8_t[size];
                    std::copy(other.val_ptr, other.val_ptr + size, val_ptr);
                }
                else
                {
                    val_ptr = other.val_ptr;
                }
            }
            return *this;
        }        

        ~Value()
        {
            if (owner)
                delete[] val_ptr;
        }

        bool is_empty() const { return type == Type::NONE; }

        template <typename T>
        T get() const
        {
            throw std::runtime_error("Invalid type");
        }

        friend bool operator==(const Value &lhs, const Value &rhs);
        friend bool operator!=(const Value &lhs, const Value &rhs);
        friend bool operator<(const Value &lhs, const Value &rhs);
        friend bool operator>(const Value &lhs, const Value &rhs);
        friend bool operator<=(const Value &lhs, const Value &rhs);
        friend bool operator>=(const Value &lhs, const Value &rhs);

        friend Value operator+(const Value& lhs, const Value& rhs);
        friend Value operator-(const Value& lhs, const Value& rhs);
        friend Value operator*(const Value& lhs, const Value& rhs);
        friend Value operator/(const Value& lhs, const Value& rhs);
        friend Value operator%(const Value& lhs, const Value& rhs);
        friend Value operator+(const Value& lhs);
        friend Value operator-(const Value& lhs);

        friend Value operator&(const Value& lhs, const Value& rhs);
        friend Value operator|(const Value& lhs, const Value& rhs);
        friend Value operator^(const Value& lhs, const Value& rhs);
        friend Value operator~(const Value& lhs);
    };

    template <>
    int32_t Value::get<int32_t>() const
    {
        if (type != Type::INT)
        {
            throw std::runtime_error("Invalid type");
        }
        return *((int32_t *)val_ptr);
    }

    template <>
    bool Value::get<bool>() const
    {
        if (type != Type::BOOL)
        {
            throw std::runtime_error("Invalid type");
        }
        return *((bool *)val_ptr);
    }

    template <>
    std::string Value::get<std::string>() const
    {
        if (type != Type::STRING)
        {
            throw std::runtime_error("Invalid type");
        }
        return std::string((const char *)val_ptr);
    }

    template <>
    Bytes Value::get<Bytes>() const
    {
        if (type != Type::BYTES)
        {
            throw std::runtime_error("Invalid type");
        }
        return Bytes(val_ptr, val_ptr + size);
    }

    bool operator==(const Value &lhs, const Value &rhs)
    {
        if (lhs.type == Type::INT)
        {
            return lhs.get<int32_t>() == rhs.get<int32_t>();
        }
        if (lhs.type == Type::BOOL)
        {
            return lhs.get<bool>() == rhs.get<bool>();
        }
        if (lhs.type == Type::STRING)
        {
            return lhs.get<std::string>() == rhs.get<std::string>();
        }
        if (lhs.type == Type::BYTES)
        {
            return lhs.get<Bytes>() == rhs.get<Bytes>();
        }
        throw std::runtime_error("Not implemented yet");
    }

    bool operator<(const Value &lhs, const Value &rhs)
    {
        if (lhs.type == Type::INT)
        {
            return lhs.get<int32_t>() < rhs.get<int32_t>();
        }
        if (lhs.type == Type::BOOL)
        {
            return lhs.get<bool>() < rhs.get<bool>();
        }
        if (lhs.type == Type::STRING)
        {
            return lhs.get<std::string>() < rhs.get<std::string>();
        }
        if (lhs.type == Type::BYTES)
        {
            return lhs.get<Bytes>() < rhs.get<Bytes>();
        }
        throw std::runtime_error("Not implemented yet");
    }

    bool operator!=(const Value &lhs, const Value &rhs)
    {
        return !(lhs == rhs);
    }

    bool operator>(const Value &lhs, const Value &rhs)
    {
        return rhs < lhs;
    }

    bool operator<=(const Value &lhs, const Value &rhs)
    {
        return !(lhs > rhs);
    }

    bool operator>=(const Value &lhs, const Value &rhs)
    {
        return !(lhs < rhs);
    }

    Value operator+(const Value& lhs, const Value& rhs)
    {
        if (lhs.type == Type::INT)
        {
            return Value(lhs.get<int32_t>() + rhs.get<int32_t>());
        }
        if (lhs.type == Type::BOOL)
        {
            throw std::runtime_error("Operation not allowed");
        }
        if (lhs.type == Type::STRING)
        {
            return Value(lhs.get<std::string>() + rhs.get<std::string>());
        }
        if (lhs.type == Type::BYTES)
        {
            throw std::runtime_error("Operation not allowed");
        }
    }

    Value operator-(const Value& lhs, const Value& rhs)
    {
        if (lhs.type == Type::INT)
        {
            return Value(lhs.get<int32_t>() - rhs.get<int32_t>());
        }
        throw std::runtime_error("Operation not allowed");
    }

    Value operator*(const Value& lhs, const Value& rhs)
    {
        if (lhs.type == Type::INT)
        {
            return Value(lhs.get<int32_t>() * rhs.get<int32_t>());
        }
        throw std::runtime_error("Operation not allowed");
    }

    Value operator/(const Value& lhs, const Value& rhs)
    {
        if (lhs.type == Type::INT)
        {
            return Value(lhs.get<int32_t>() / rhs.get<int32_t>());
        }
        throw std::runtime_error("Operation not allowed");
    }

    Value operator%(const Value& lhs, const Value& rhs)
    {
        if (lhs.type == Type::INT)
        {
            return Value(lhs.get<int32_t>() % rhs.get<int32_t>());
        }
        throw std::runtime_error("Operation not allowed");
    }

    Value operator+(const Value& lhs)
    {
        if (lhs.type == Type::INT)
        {
            return lhs;
        }
        throw std::runtime_error("Operation not allowed");
    }

    Value operator-(const Value& lhs)
    {
        if (lhs.type == Type::INT)
        {
            return Value(-lhs.get<int32_t>());
        }
        throw std::runtime_error("Operation not allowed");
    }

    Value operator&(const Value& lhs, const Value& rhs)
    {
        if (lhs.type == Type::BOOL)
        {
            return Value(lhs.get<bool>() && rhs.get<bool>());
        }
        throw std::runtime_error("Operation not allowed");
    }

    Value operator|(const Value& lhs, const Value& rhs)
    {
        if (lhs.type == Type::BOOL)
        {
            return Value(lhs.get<bool>() || rhs.get<bool>());
        }
        throw std::runtime_error("Operation not allowed");
    }

    Value operator^(const Value& lhs, const Value& rhs)
    {
        if (lhs.type == Type::BOOL)
        {
            return Value(lhs.get<bool>() != rhs.get<bool>());
        }
        throw std::runtime_error("Operation not allowed");
    }

    Value operator~(const Value& lhs)
    {
        if (lhs.type == Type::BOOL)
        {
            return Value(!lhs.get<bool>());
        }
        throw std::runtime_error("Operation not allowed");
    }

}
