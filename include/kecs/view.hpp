#pragma once

#include "deserialize.hpp"
#include <spdlog/spdlog.h>

namespace kecs
{

// ================================================================
// 7) Zero-Copy View
// ================================================================
template <typename Tuple>
class View
{
    K data;

public:
    View(K d) : data(d) {}

    ~View()
    {
        if(data)
        {
            r0(data);
        }
    }

    struct Iterator
    {
        K     data;
        J     idx;
        bool  operator!=(const Iterator& o) const { return idx != o.idx; }
        void  operator++() { ++idx; }
        Tuple operator*() const { return deserialize_tuple<Tuple>(data, idx); }
    };

    auto begin()
    {
        return Iterator{data, 0};
    }

    auto end()
    {
        if(!data)
        {
            return Iterator{data, 0};
        }

        if(data->t != XT)
        {
            spdlog::error("unexpected K type, not a table!");
            return Iterator{data, 0};
        }

        K col_data = kK(data->k)[1];
        J row_count = (col_data->n > 0) ? kK(col_data)[0]->n : 0;

        return Iterator{data, row_count};
    }
};


} // namespace kecs
