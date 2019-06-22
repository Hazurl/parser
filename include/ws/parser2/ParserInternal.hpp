#pragma once

#include <variant>
#include <tuple>
#include <string>
#include <iostream>

#include <module/module.h>

#include <ws/parser2/Details.hpp>
#include <ws/parser2/Storage.hpp>

namespace ws::parser2 {

template<typename...Args>
decltype(auto) success(std::size_t cursor, Args&&...args) {
    return ResultBuilder{ [&] (auto r) { 
        using T = typename decltype(r)::type;
        return T(std::in_place_index_t<0>{}, cursor, std::forward<Args>(args)...); 
    }};
}

template<typename E, typename...Args>
decltype(auto) fail(std::size_t cursor, Args&&...args) {
    return ResultBuilder{ [&] (auto r) { 
        using T = typename decltype(r)::type;
        static_assert(details::is_in_v<E, details::list_from_errors_t<T>>, "Failure type mismatch, error isn't in the result");
        return T(std::in_place_index_t<details::index_of_v<details::list_from_errors_t<T>, E> + 1>{}, cursor, std::forward<Args>(args)...); 
    }};
}

namespace error {

struct EndOfFile {
    std::string what() const {
        return "End of file reached!";
    }
};

}

template<typename C>
struct Next : Parser<Next<C>, C, error::EndOfFile> {
    template<typename R>
    static Result<C, error::EndOfFile> parse(R reader) {
        if (reader.empty()) {
            return fail<error::EndOfFile>(reader.cursor);
        }

        return success(reader.cursor, reader.peek());
    }
};

template<typename C>
constexpr Next<C> next;
constexpr Next<char> nextc;

template<typename P, std::enable_if_t<details::is_parser_soft_check_v<P>, bool> = true>
struct Opt : Parser<Opt<P>, Maybe<details::parsed_type_t<P>>> {
    template<typename R>
    static details::result_type_t<Opt<P>> parse(R reader) {
        auto res = P::parse(reader);

        if (res.is_error()) {
            return success(reader.cursor, std::nullopt);
        }

        return success(reader.cursor, std::move(res.success()));
    }

};

template<auto& p>
constexpr Opt<std::decay_t<decltype(p)>> opt;

}