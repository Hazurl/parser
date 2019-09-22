#pragma once

#include <ws/parser2/Parsers.hpp>
#include <ws/parser2/containers/Parser.hpp>

namespace ws::parser2 {

template<auto& F>
struct BindErr {};

template<auto& F>
constexpr BindErr<F> bind_err;


/*
    Handle transformer
    -- if `P` returns an error, handle it with `F`, it must returns the success type of `P`
 */

template<typename P, auto& F>
struct ErrorBinder : Parser<
    ErrorBinder<P, F>, 
    details::parsed_type_t<P>, 
    details::error_of_t<std::invoke_result_t<decltype(F), details::error_of_t<details::result_type_t<P>>>>
> {
    static_assert(details::is_parser_soft_check_v<P>, "First parameter of ErrorBinder must be a parser");
    static_assert(P::can_fail, "The parser can't fail, thus you can't handle its error");
    static_assert(std::is_invocable_v<decltype(F), details::error_of_t<details::result_type_t<P>>>, "ErrorBinder is not invokable with the parser's error type");
    static_assert(details::is_same_HK_type_v<std::invoke_result_t<decltype(F), details::error_of_t<details::result_type_t<P>>>, Result>, "ErrorBinder's function must returns a Result");
    static_assert(std::is_constructible_v<details::parsed_type_t<P>, details::success_of_t<std::invoke_result_t<decltype(F), details::error_of_t<details::result_type_t<P>>>>>, 
        "Handler's return type can't construct the return type of the parser");

    template<typename R>
    static details::result_type_t<ErrorBinder<P, F>> parse(R r) {
        auto res = P::parse(r);

        if (res.is_error()) {
            auto f_res = F(std::move(res.error()));
            if (f_res.is_error()) {
                return fail(res.cursor(), std::move(f_res.error()));
            }
            return success(res.cursor(), std::move(f_res).success());
        }

        return success(res.cursor(), std::move(res).success());
    }
};


template<auto P, auto& F>
constexpr ErrorBinder<std::decay_t<decltype(P)>, F> error_binder;






namespace details {

template<auto& F, typename P>
struct GetTransformer<::ws::parser2::BindErr<F>, P> {
    using type = ErrorBinder<P, F>;
};

}

}