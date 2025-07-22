#pragma once

#include "ast.hpp"
#include <algorithm>
#include <boost/pfr.hpp>
#include <cxxabi.h>
#include <string>
#include <typeinfo>

namespace kecs
{

// ================================================================
// Demangle
// ================================================================
template <typename Type>
std::string demangle()
{
    int         status;
    char*       realname = abi::__cxa_demangle(typeid(Type).name(), 0, 0, &status);
    std::string res = (status == 0 ? realname : typeid(Type).name());
    free(realname);

    res.erase(std::remove_if(res.begin(), res.end(), [](char c)
    {
        return !isalnum(c);
    }),
              res.end());

    std::transform(res.begin(), res.end(), res.begin(), ::tolower);
    return res;
}

// ================================================================
// Query Builders
// ================================================================

// 1) Fallback (plain components)
template <typename Type>
std::string to_query()
{
    if constexpr(!is_query_expression_v<Type>)
        return demangle<Type>();
    else
        return to_query(Type{});
}

// 2) Fields
template <typename Component, FixedString Number>
std::string to_query(field<Component, Number>)
{
    return demangle<Component>() + "." + std::string(Number);
}

// 3) Comparisons
template <typename Field, auto Value>
std::string to_query(Greater<Field, Value>)
{
    return to_query<Field>() + " > " + std::to_string(Value);
}
template <typename Field, auto Value>
std::string to_query(Less<Field, Value>)
{
    return to_query<Field>() + " < " + std::to_string(Value);
}
template <typename Field, auto Value>
std::string to_query(Equal<Field, Value>)
{
    return to_query<Field>() + " = " + std::to_string(Value);
}

// 4) Aggregates
template <typename Field>
std::string to_query(sum<Field>) { return "sum " + to_query<Field>(); }
template <typename Field>
std::string to_query(avg<Field>) { return "avg " + to_query<Field>(); }
template <typename Field>
std::string to_query(min<Field>) { return "min " + to_query<Field>(); }
template <typename Field>
std::string to_query(max<Field>) { return "max " + to_query<Field>(); }
template <typename Field>
std::string to_query(count<Field>) { return "count " + to_query<Field>(); }

// 5) Logical
template <typename Left, typename Right>
std::string to_query(And<Left, Right>)
{
    return "(" + to_query<Left>() + " and " + to_query<Right>() + ")";
}
template <typename Left, typename Right>
std::string to_query(Or<Left, Right>)
{
    return "(" + to_query<Left>() + " or " + to_query<Right>() + ")";
}
template <typename Type>
std::string to_query(Not<Type>)
{
    return "not " + to_query<Type>();
}

} // namespace kecs
