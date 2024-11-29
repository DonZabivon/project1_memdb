#pragma once

#include <stdexcept>
#include <vector>

namespace memdb
{

    struct OrderedIndex
    {
        size_t col;
        std::vector<size_t> index;

        OrderedIndex(size_t col) : col(col) {}

        friend bool operator<(const OrderedIndex& x, const OrderedIndex& y)
        {
            return x.col < y.col;
        }
    };

    struct IndexRange
    {
        const OrderedIndex* index;
        size_t begin;
        size_t end;

        IndexRange(const OrderedIndex* index, size_t begin, size_t end) : index(index), begin(begin), end(end) {}

        size_t size() const { return end - begin; }

        static bool compare_by_size(const IndexRange& x, const IndexRange& y)
        {
            return x.size() < y.size();
        }

        friend bool operator<(const IndexRange& x, const IndexRange& y)
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
}
