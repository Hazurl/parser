#pragma once

#include <wpr/Parsers.hpp>
#include <wpr/containers/Parser.hpp>

namespace wpr {

template<auto& F>
struct Handler {};

template<auto& F>
constexpr Handler<F> handler;


/*
    Handle transformer
    -- if `P` returns an error, handle it with `F`, it must returns the success type of `P`
 */

template<typename P, auto& F>
struct Handle : Parser<Handle<P, F>, details::parsed_type_t<P>> {
    static_assert(details::is_parser_soft_check_v<P>, "First parameter of Handle must be a parser");
    static_assert(P::can_fail, "The parser can't fail, thus you can't handle its error");
    static_assert(std::is_invocable_v<decltype(F), details::error_of_t<details::result_type_t<P>>>, "Handler is not invokable with the parser's error type");
    static_assert(std::is_constructible_v<details::parsed_type_t<P>, std::invoke_result_t<decltype(F), details::error_of_t<details::result_type_t<P>>>>, 
        "Handler's return type can't construct the return type of the parser");

    template<typename R>
    static details::result_type_t<Handle<P, F>> parse(R r) {
        auto res = P::parse(r);

        if (res.is_error()) {
            return success(res.cursor(), F(std::move(res).error()));
        }

        return success(res.cursor(), std::move(res).success());
    }
};


template<auto P, auto& F>
constexpr Handle<std::decay_t<decltype(P)>, F> handle;






namespace details {

template<auto& F, typename P>
struct GetTransformer<::wpr::Handler<F>, P> {
    using type = Handle<P, F>;
};

}

}