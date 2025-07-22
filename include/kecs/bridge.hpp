#pragma once

#include "traits.hpp"
#include "utils.hpp"
#include "view.hpp"
#include <spdlog/spdlog.h>

namespace kecs
{


// ================================================================
// 8) ECS Core (Shared Memory Ready)
// ================================================================
class bridge
{
    int    m_handle;
    entity nextEntity = 1;

public:
    bridge(const std::string& src = "127.0.0.1", int port = 5000)
    {
        m_handle = khp(const_cast<S>(src.c_str()), port);

        if(m_handle <= 0)
        {
            throw std::runtime_error("Failed to connect");
        }
    }

    ~bridge()
    {
        if(m_handle > 0)
        {
            kclose(m_handle);
        }
    }

    entity create_entity() { return nextEntity++; }

    template <typename... Cs>
    entity create(const Cs&... comps)
    {
        entity entity = create_entity();
        (add_component(entity, comps), ...);
        return entity;
    }

    template <typename Component>
    void add_component(entity entity, const Component& comp)
    {
        ensure_table<Component>();

        std::string query = "`" + demangle<Component>() + " insert (" + std::to_string(entity);

        boost::pfr::for_each_field(comp, [&](const auto& f, std::size_t)
        {
            query += ";" + field_to_string(f);
        });

        query += ")";

        spdlog::debug("insert data: {}", query);

        k(m_handle, (S)query.c_str(), (K)0);
    }

    template <typename Component>
    void ensure_table()
    {
        static bool created = false;
        if(!created)
        {
            std::string query = table_schema<Component>();
            spdlog::debug("create table: {}", query);

            k(m_handle, (S)query.c_str(), (K)0);
            created = true;
        }
    }

    template <typename Component>
    std::string table_schema()
    {
        std::string schema = demangle<Component>() + ":([] id:`long$()";
        auto        names = boost::pfr::names_as_array<Component>();
        boost::pfr::for_each_field(Component{}, [&](const auto& f, std::size_t i)
        {
            using FT = std::remove_cvref_t<decltype(f)>;
            schema += "; " + std::string(names[i]) + ":" + kdb_type<FT>() + "$()";
        });
        schema += ")";
        return schema;
    }

    template <typename Type>
    std::string kdb_type()
    {
        if constexpr(std::is_same_v<Type, int>)
        {
            return "`int";
        }
        else if constexpr(std::is_same_v<Type, float>)
        {
            return "`float";
        }
        else if constexpr(std::is_same_v<Type, double>)
        {
            return "`float";
        }
        else
        {
            return "`symbol";
        }
    }

    template <typename Type>
    std::string field_to_string(const Type& v)
    {
        if constexpr(std::is_same_v<Type, std::string>)
        {
            return "`" + v;
        }
        else
        {
            return std::to_string(v);
        }
    }

    template <typename Expr>
    auto view()
    {
        using Raw = extract_components_t<Expr>;
        using Comps = flatten_tuple_t<Raw>;

        std::string query = "select id" + fieldList<Comps>() +
                            " from " + join_tables<Comps>();

        if constexpr(is_field_expression_v<Expr>)
        {
            query += " where " + to_query(Expr{});
        }

        spdlog::debug("view: {}", query);

        K r = k(m_handle, (S)query.c_str(), (K)0);

        spdlog::debug("created view");

        using ResultTuple = decltype(std::tuple_cat(std::tuple<entity>{}, Comps{}));
        return View<ResultTuple>(r);
    }

    template <typename AggExpr>
    auto aggregate()
    {
        using Raw = extract_components_t<AggExpr>;
        using Comps = flatten_tuple_t<Raw>;
        std::string query = "select " + to_query(AggExpr{}) + " from " + join_tables<Comps>();

        spdlog::debug("aggregate: {}", query);

        K    r = k(m_handle, (S)query.c_str(), (K)0);
        auto val = kF(kK(r->k)[0])[0];
        r0(r);
        return val;
    }

private:
    template <typename Tuple>
    static std::string fieldList()
    {
        if constexpr(std::tuple_size_v<Tuple> == 0)
        {
            return "";
        }

        std::string s;
        std::apply([&](auto... c)
        {
            ((boost::pfr::for_each_field(c, [&](const auto&, std::size_t i)
            {
                auto names = boost::pfr::names_as_array<std::decay_t<decltype(c)>>();
                s += "," + std::string(names[i]);
            })),
             ...);
        }, Tuple{});
        return s;
    }

    template <typename Tuple>
    static std::string join_tables()
    {
        if constexpr(std::tuple_size_v<Tuple> == 1)
        {
            using C = std::tuple_element_t<0, Tuple>;
            return demangle<C>();
        }
        else
        {
            std::string s = "ej[`id;";
            std::size_t n = 0;
            std::apply([&](auto... c)
            {
                ((s += (n++ == 0 ? "" : ";") + demangle<std::decay_t<decltype(c)>>()), ...);
            }, Tuple{});
            s += "]";
            return s;
        }
    }
};

} // namespace kecs