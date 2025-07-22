#pragma once

#include "types.hpp"
#include <boost/pfr.hpp>
#include <string>
#include <type_traits>

extern "C"
{
#include "c/k.h"
}

namespace kecs
{
// ================================================================
// 6) Deserialization
// ================================================================

// Helper: number of KDB+ columns consumed by a type
// entity occupies 1 column; each struct component occupies as many columns as its fields
template <typename T>
constexpr std::size_t col_count_v =
    std::is_same_v<T, entity>
        ? 1
        : boost::pfr::tuple_size_v<T>;

// offset_for<Tuple, I> = sum of col_count_v<Elem<0..I-1>>
template <typename Tuple, std::size_t I>
struct offset_for
{
    static constexpr std::size_t value =
        offset_for<Tuple, I - 1>::value + col_count_v<std::tuple_element_t<I - 1, Tuple>>;
};

template <typename Tuple>
struct offset_for<Tuple, 0>
{
    static constexpr std::size_t value = 0;
};

// Deserialize a single row’s value of type Type from the K table at given column offset
template <typename Type>
Type deserialize_row(K table, J rowIndex, std::size_t offset)
{
    if constexpr(std::is_same_v<Type, entity>)
    {
        // entity is in the single column at position 'offset'
        K col = kK(kK(table->k)[1])[offset];
        return kJ(col)[rowIndex];
    }
    else
    {
        Type obj{};
        auto names = boost::pfr::names_as_array<Type>();

        // For each field of the component, extract from (offset + fieldIndex)
        boost::pfr::for_each_field(obj, [&](auto& field, std::size_t i)
        {
            K col = kK(kK(table->k)[1])[offset + i];
            using FT = std::remove_cvref_t<decltype(field)>;
            if constexpr(std::is_same_v<FT, int>)
                field = kI(col)[rowIndex];
            else if constexpr(std::is_same_v<FT, double>)
                field = kF(col)[rowIndex];
            else if constexpr(std::is_same_v<FT, float>)
                field = static_cast<float>(kF(col)[rowIndex]);
            else if constexpr(std::is_same_v<FT, std::string>)
                field = std::string(kS(col)[rowIndex]);
        });

        return obj;
    }
}

// Deserialize a tuple by calling deserialize_row for each element with its computed offset
template <typename Tuple, std::size_t... I>
Tuple deserialize_tuple_impl(K table, J row, std::index_sequence<I...>)
{
    return Tuple{
        deserialize_row<std::tuple_element_t<I, Tuple>>(table, row, offset_for<Tuple, I>::value)...};
}

// Public API: deserialize a tuple from a K table row
template <typename Tuple>
Tuple deserialize_tuple(K table, J row)
{
    return deserialize_tuple_impl<Tuple>(
        table,
        row,
        std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

} // namespace kecs