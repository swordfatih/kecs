#pragma once

#include "ast.hpp"
#include <tuple>
#include <type_traits>

namespace kecs
{

// ================================================================
// Trait: is_query_expression
// ================================================================
template <typename Type>
struct is_query_expression : std::false_type
{
};

// Logical operators
template <typename Left, typename Right>
struct is_query_expression<And<Left, Right>> : std::true_type
{
};

template <typename Left, typename Right>
struct is_query_expression<Or<Left, Right>> : std::true_type
{
};

template <typename Type>
struct is_query_expression<Not<Type>> : std::true_type
{
};

// Comparisons
template <typename field, auto Value>
struct is_query_expression<Greater<field, Value>> : std::true_type
{
};

template <typename field, auto Value>
struct is_query_expression<Less<field, Value>> : std::true_type
{
};

template <typename field, auto Value>
struct is_query_expression<Equal<field, Value>> : std::true_type
{
};

// Aggregates
template <typename field>
struct is_query_expression<sum<field>> : std::true_type
{
};

template <typename field>
struct is_query_expression<avg<field>> : std::true_type
{
};

template <typename field>
struct is_query_expression<min<field>> : std::true_type
{
};

template <typename field>
struct is_query_expression<max<field>> : std::true_type
{
};

template <typename field>
struct is_query_expression<count<field>> : std::true_type
{
};

template <typename Type>
inline constexpr bool is_query_expression_v = is_query_expression<Type>::value;

// ================================================================
// Extract Components from Queries
// ================================================================

// Default: no components
template <typename Type, typename = void>
struct extract_components
{
    using type = std::tuple<>;
};

// Components (exclude query expressions)
template <typename Type>
struct extract_components<
    Type,
    std::enable_if_t<std::is_class_v<Type> && !is_query_expression_v<Type>>>
{
    using type = std::tuple<Type>;
};

// Fields
template <typename Component, FixedString Number>
struct extract_components<field<Component, Number>>
{
    using type = std::tuple<Component>;
};

// Aggregates forward to their field
template <template <typename> class Agg, typename field>
struct extract_components<Agg<field>> : extract_components<field>
{
};

// Logical expressions
template <typename Left, typename Right>
struct extract_components<And<Left, Right>>
{
    using type = decltype(std::tuple_cat(
        std::declval<typename extract_components<Left>::type>(),
        std::declval<typename extract_components<Right>::type>()));
};

template <typename Left, typename Right>
struct extract_components<Or<Left, Right>> : extract_components<And<Left, Right>>
{
};

template <typename Type>
struct extract_components<Not<Type>> : extract_components<Type>
{
};

template <typename Type>
using extract_components_t = typename extract_components<Type>::type;

// ================================================================
// flatten_tuple: list types
// ================================================================
template <typename... Ts>
struct flatten_tuple;

template <>
struct flatten_tuple<>
{
    using type = std::tuple<>;
};

template <typename... Ts>
struct flatten_tuple<std::tuple<Ts...>>
{
    using type = typename flatten_tuple<Ts...>::type;
};

template <typename Type, typename... Ts>
struct flatten_tuple<Type, Ts...>
{
    using type = decltype(std::tuple_cat(std::tuple<Type>{}, typename flatten_tuple<Ts...>::type{}));
};

template <typename... Ts>
using flatten_tuple_t = typename flatten_tuple<Ts...>::type;

// ================================================================
// Trait: is_field_expression
// ================================================================
template <typename Type>
struct is_field_expression : std::false_type
{
};

template <typename Component, FixedString Number>
struct is_field_expression<field<Component, Number>> : std::true_type
{
};

template <typename Type>
inline constexpr bool is_field_expression_v = is_field_expression<Type>::value;


} // namespace kecs
