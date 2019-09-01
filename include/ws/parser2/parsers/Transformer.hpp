#pragma once

#include <type_traits>

#include <ws/parser2/containers/Parser.hpp>
#include <ws/parser2/Details.hpp>

namespace ws::parser2 {

/*
    Used in operator[] to get a compile-time callable
*/
template<auto& F>
struct Map {};

template<auto& F>
constexpr Map<F> map;





template<typename P, auto& F>
struct Transformer : Parser<Transformer<P, F>, std::invoke_result_t<decltype(F), details::parsed_type_t<P>>, details::error_of_t<details::result_type_t<P>>> {
    static_assert(details::is_parser_soft_check_v<P>, "First parameter of Transformer must be a parser");
    static_assert(std::is_invocable_v<decltype(F), details::parsed_type_t<P>>, "Function is not invokable with the parser's return type");

    template<typename R>
    static details::result_type_t<Transformer<P, F>> parse(R r) {
        auto res = P::parse(r);
        if constexpr(P::can_fail) {
            if (res.is_error()) {
                return fail(res.cursor(), std::move(res).error());
            }
        }
        return success(res.cursor(), F(std::move(res).success()));
    }
};

template<auto P, auto& F>
constexpr Transformer<std::decay_t<decltype(P)>, F> transformer;




namespace details {

template<auto& F, typename P>
struct GetTransformer<::ws::parser2::Map<F>, P> {
    using type = Transformer<P, F>;
};

}

}