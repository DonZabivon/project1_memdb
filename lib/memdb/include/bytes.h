#pragma once

#include <vector>
#include <stdexcept>
#include <iomanip>
#include <sstream>

#include "base.h"

namespace memdb
{

    typedef std::vector<uint8_t> Bytes;

    Bytes bytes_from_hex_string(const std::string &str)
    {
        Bytes bytes;
        for (size_t i = 2; i < str.size(); i += 2)
        {
            uint8_t byte = (uint8_t)stoul(str.substr(i, 2), 0, 16);
            bytes.push_back(byte);
        }
        return bytes;
    }

    std::string to_string(const Bytes &bytes)
    {
        std::ostringstream oss;
        oss << "0x";
        for (const auto &byte : bytes)
        {
            oss << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)byte;
        }
        return oss.str();
    }

}
