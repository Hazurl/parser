#pragma once

#include <type_traits>

#include <ws/parser2/containers/Parser.hpp>
#include <ws/parser2/Details.hpp>

namespace ws::parser2 {

/*
    Used in operator[] to get a compile-time callable
*/
template<auto& F>
struct MapErr {};

template<auto& F>
constexpr MapErr<F> map_err;





template<typename P, auto& F>
struct ErrorTransformer : Parser<ErrorTransformer<P, F>, details::parsed_type_t<P>, std::invoke_result_t<decltype(F), details::error_of_t<details::result_type_t<P>>>> {
    static_assert(details::is_parser_soft_check_v<P>, "First parameter of ErrorTransformer must be a parser");
    static_assert(P::can_fail, "Parser can't fail, so it can't be transformed");
    static_assert(std::is_invocable_v<decltype(F), details::error_of_t<details::result_type_t<P>>>, "Function is not invokable with the parser's error type");

    template<typename R>
    static details::result_type_t<ErrorTransformer<P, F>> parse(R r) {
        auto res = P::parse(r);
        if constexpr(P::can_fail) {
            if (res.is_error()) {
                return fail(res.cursor(), F(std::move(res).error()));
            }
        }
        return success(res.cursor(), std::move(res).success());
    }
};

template<auto P, auto& F>
constexpr ErrorTransformer<std::decay_t<decltype(P)>, F> error_transformer;




namespace details {

template<auto& F, typename P>
struct GetTransformer<::ws::parser2::MapErr<F>, P> {
    using type = ErrorTransformer<P, F>;
};

}

}