#pragma once

#include <stdexcept>
#include <string>
#include <iostream>

#include "base.h"
#include "bytes.h"

namespace memdb
{
	inline bool strcmpi(const std::string &a, const std::string &b)
	{
		if (a.length() != b.length())
			return false;

		for (size_t i = 0; i < a.length(); ++i)
		{
			if (tolower(a[i]) != tolower(b[i]))
				return false;
		}

		return true;
	}

	template <typename T>
	inline void write_int(std::ostream &out, T val)
	{
		out.write((const char *)&val, sizeof(val));
	}

	inline void write_string(std::ostream &out, const std::string &val)
	{
		size_t n = val.size();
		write_int(out, n);
		out.write((const char *)val.c_str(), n);
	}

	inline void write_bytes(std::ostream &out, const Bytes &val)
	{
		size_t n = val.size();
		write_int(out, n);
		out.write((const char *)val.data(), n);
	}

	template <typename T>
	inline T read_int(std::istream &in)
	{
		T val;
		in.read((char *)&val, sizeof(val));
		return val;
	}

	inline std::string read_string(std::istream &in)
	{
		size_t n = read_int<size_t>(in);
		char *data = new char[n];
		in.read(data, n);
		std::string s = std::string(data, data + n);
		delete[] data;
		return s;
	}

	inline Bytes read_bytes(std::istream &in)
	{
		size_t n = read_int<size_t>(in);
		uint8_t *data = new uint8_t[n];
		in.read((char *)data, n);
		Bytes b = Bytes(data, data + n);
		delete[] data;
		return b;
	}
}
