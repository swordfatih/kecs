#pragma once

#include "types.hpp"

namespace kecs
{

// ================================================================
// Query AST (Logical + Comparisons)
// ================================================================
template <typename Left, typename Right>
struct And
{
    using left = Left;
    using right = Right;
};
template <typename Left, typename Right>
struct Or
{
    using left = Left;
    using right = Right;
};
template <typename Type>
struct Not
{
    using inner = Type;
};

template <typename field, auto Value>
struct Greater
{
    using Field = field;
    static constexpr auto value = Value;
};
template <typename field, auto Value>
struct Less
{
    using Field = field;
    static constexpr auto value = Value;
};
template <typename field, auto Value>
struct Equal
{
    using Field = field;
    static constexpr auto value = Value;
};

template <typename field>
struct sum
{
    using Field = field;
};
template <typename field>
struct avg
{
    using Field = field;
};
template <typename field>
struct min
{
    using Field = field;
};
template <typename field>
struct max
{
    using Field = field;
};
template <typename field>
struct count
{
    using Field = field;
};

// ================================================================
// Operators
// ================================================================
template <typename Left, typename Right>
constexpr auto operator&&(Left, Right) { return And<Left, Right>{}; }
template <typename Left, typename Right>
constexpr auto operator||(Left, Right) { return Or<Left, Right>{}; }
template <typename Type>
constexpr auto operator!(Type) { return Not<Type>{}; }
template <typename field, auto Value>
constexpr auto operator>(field, _c<Value>) { return Greater<field, Value>{}; }
template <typename field, auto Value>
constexpr auto operator<(field, _c<Value>) { return Less<field, Value>{}; }
template <typename field, auto Value>
constexpr auto operator==(field, _c<Value>) { return Equal<field, Value>{}; }

} // namespace kecs
