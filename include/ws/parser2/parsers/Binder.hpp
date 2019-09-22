#pragma once

#include <ws/parser2/Parsers.hpp>
#include <ws/parser2/containers/Parser.hpp>

namespace ws::parser2 {

namespace {

/*
    Parser<_> => Parser<_, E>
    Parser<_, E> => Parser<_, E>
    Parser<_, R> => Parser<_, Sum<E, R>>
*/

template<typename R, typename E>
using error_t = 
    std::conditional_t<
        /* if   */ std::is_same_v<R, void> || std::is_same_v<R, E>, 
        /* then */ E,
        /* else */ Sum<E, R>
    >;

}

template<auto& F>
struct Bind {};

template<auto& F>
constexpr Bind<F> bind;


/*
    Handle transformer
    -- if `P` returns an error, handle it with `F`, it must returns the success type of `P`
 */

template<typename P, auto& F>
struct Binder : Parser<
    Binder<P, F>, 
    details::success_of_t<std::invoke_result_t<decltype(F), details::parsed_type_t<P>>>, 
    error_t<details::error_of_t<details::result_type_t<P>>, details::error_of_t<std::invoke_result_t<decltype(F), details::parsed_type_t<P>>>>
> {
    static_assert(details::is_parser_soft_check_v<P>, "First parameter of Binder must be a parser");
    static_assert(std::is_invocable_v<decltype(F), details::parsed_type_t<P>>, "Binder is not invokable with the parser's return type");
    static_assert(details::is_same_HK_type_v<std::invoke_result_t<decltype(F), details::parsed_type_t<P>>, Result>, "Binder's function must returns a Result");

    template<typename R>
    static details::result_type_t<Binder<P, F>> parse(R r) {
        auto res = P::parse(r);

        if constexpr (P::can_fail) {
            if (res.is_error()) {
                if constexpr (!std::is_same_v<details::error_of_t<details::result_type_t<P>>, details::error_of_t<std::invoke_result_t<decltype(F), details::parsed_type_t<P>>>> ) {
                    return fail(res.cursor(), std::in_place_index_t<0>{}, std::move(res.error()));
                } else {
                    return fail(res.cursor(), std::move(res.error()));
                }
            }
        }

        auto f_res = F(std::move(res.success()));
        if (f_res.is_error()) {
            if constexpr (P::can_fail && !std::is_same_v<details::error_of_t<details::result_type_t<P>>, details::error_of_t<std::invoke_result_t<decltype(F), details::parsed_type_t<P>>>> ) {
                return fail(res.cursor(), std::in_place_index_t<1>{}, std::move(f_res.error()));
            } else {
                return fail(res.cursor(), std::move(f_res.error()));
            }
        }

        return success(res.cursor(), std::move(f_res.success()));
    }
};


template<auto P, auto& F>
constexpr Binder<std::decay_t<decltype(P)>, F> cinder;






namespace details {

template<auto& F, typename P>
struct GetTransformer<::ws::parser2::Bind<F>, P> {
    using type = Binder<P, F>;
};

}

}