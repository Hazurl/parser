#pragma once

#include <wpr/Parsers.hpp>
#include <wpr/containers/Parser.hpp>

namespace wpr {

/*
    Catcher parser
    -- Run P, if E is raised, catch it and sum it with the error
 */

template<typename P, typename E>
struct Catcher : Parser<Catcher<P, E>, details::parsed_type_t<P>, 
    std::conditional_t<P::can_fail, Sum<details::error_of_t<details::result_type_t<P>>, E>, E>
> {
    static_assert(details::is_parser_soft_check_v<P>, "First parameter of Catcher must be a parser");

    template<typename R>
    static details::result_type_t<Catcher<P, E>> parse(R r) {
        try {
            auto res = P::parse(r);

            if constexpr (P::can_fail) {
                if (res.is_error()) {
                    return fail(res.cursor(), std::in_place_index_t<0>{}, std::move(res).error());
                }
            }

            return success(res.cursor(), std::move(res).success());
        } catch (E err) {
            if constexpr (P::can_fail) {
                return fail(r.cursor(), std::in_place_index_t<1>{}, std::move(err));
            } else {
                return fail(r.cursor(), std::move(err));
            }
        }
    }
};

template<auto P, typename E>
constexpr Catcher<std::decay_t<decltype(P)>, E> catcher;

}