#pragma once

#include <ws/parser2/Containers.hpp>
#include <ws/parser2/Details.hpp>
#include <ws/parser2/parsers/Errors.hpp>

namespace ws::parser2 {

/*
    Opt parser
    -- `P` is the parser tried
    -- is always successful
    -- return null when the underlying parser `P` failed
 */

template<typename P, std::enable_if_t<details::is_parser_soft_check_v<P>, int> = 1337>
struct Opt : Parser<Opt<P>, Maybe<details::parsed_type_t<P>> > {
    template<typename R>
    static details::result_type_t<Opt<P>> parse(R reader) {
        auto res = P::parse(reader);

        if (res.is_error()) {
            return success(reader.cursor(), std::nullopt);
        }

        return success(res.cursor(), std::move(res.success()));
    }

};






/*
    Parser as value
 */

template<auto p>
constexpr Opt<std::decay_t<decltype(p)>> opt;

}