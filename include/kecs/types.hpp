#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace kecs
{

using entity = uint64_t;

// ================================================================
// FixedString + field<> + _c
// ================================================================
template <size_t Number>
struct FixedString
{
    char data[Number];
    constexpr FixedString(const char (&s)[Number]) { std::copy_n(s, Number, data); }
    constexpr operator std::string_view() const { return {data, Number - 1}; }
};

template <typename Component, FixedString Number>
struct field
{
    using component = Component;
    static constexpr auto name = Number;
};

template <auto Value>
using _c = std::integral_constant<decltype(Value), Value>;

} // namespace kecs
