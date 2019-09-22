#pragma once

#include <wpr/Parsers.hpp>
#include <wpr/parsers/Handle.hpp>
#include <wpr/containers/Maybe.hpp>
#include <wpr/containers/Parser.hpp>

namespace wpr {

namespace {

/*
    Parser<_> => Parser<_, PredicateFailure>
    Parser<_, PredicateFailure> => Parser<_, PredicateFailure>
    Parser<_, E> => Parser<_, Sum<PredicateFailure, E>>
*/

template<typename R, typename E>
using error_t = 
    std::conditional_t<
        /* if   */ std::is_same_v<R, void> || std::is_same_v<R, E>, 
        /* then */ E,
        /* else */ Sum<E, R>
    >;

}

template<auto& F, typename E = error::PredicateFailure>
struct Filter {};

template<auto& F, typename E = error::PredicateFailure>
constexpr Filter<F, E> filter;


/*
    Satisfy parser
    -- Run the parser `P`, if it succeed the predicate is evaluated with the success value, if it fail `E` is returned, otherwise the success value is returned
 */

template<typename P, auto& F, typename E = error::PredicateFailure>
struct Satisfy : Parser<Satisfy<P, F, E>, details::parsed_type_t<P>, error_t<details::error_of_t<details::result_type_t<P>>, E>> {
    static_assert(details::is_parser_soft_check_v<P>, "First parameter of Satisfy must be a parser");
    static_assert(std::is_invocable_v<decltype(F), details::parsed_type_t<P>>, "Function is not invokable with the parser's return type");

    template<typename R>
    static details::result_type_t<Satisfy<P, F, E>> parse(R r) {
        auto res = P::parse(r);
        using res_t = decltype(res);
        using const_res_t = std::add_const_t<res_t>;
        using const_ref_res_t = std::add_lvalue_reference_t<const_res_t>;

        if constexpr(P::can_fail) {
            if (res.is_error()) {
                return fail(r.cursor(), std::move(res).error());
            }
        }

        if (F(static_cast<const_ref_res_t>(res).success())) {
            return success(res.cursor(), std::move(res).success());
        } else {
            return fail(r.cursor(), E{});
        }
    }
};


template<auto P, auto& F, typename E = error::PredicateFailure>
constexpr Satisfy<std::decay_t<decltype(P)>, F, E> satisfy;






namespace {

template<auto V>
bool is_equals(decltype(V) const& o) {
    return V == o;
}

}

template<auto V, auto P = next<decltype(V)>>
constexpr Satisfy<decltype(P), is_equals<V>, error::NotMatching<V>> ch;






namespace details {

template<auto& F, typename E, typename P>
struct GetTransformer<::wpr::Filter<F, E>, P> {
    using type = Satisfy<P, F, E>;
};

}


}
