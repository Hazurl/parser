#pragma once

#include <type_traits>

#include <wpr/Parsers.hpp>

namespace wpr {

namespace {

template<typename E, auto V>
auto or_else_helper(E) {
    return V;
}

template<typename E, auto F>
auto or_elsef_helper(E) {
    return F();
}

}

template<auto V>
struct OrElse {};

template<auto V>
constexpr OrElse<V> or_else;

template<auto F>
struct OrElseF {};

template<auto F>
constexpr OrElseF<F> or_elsef;


namespace details {

template<auto V, typename P>
struct GetTransformer<OrElse<V>, P> {
    using error_t = details::error_of_t<details::result_type_t<P>>;
    using type = Handle<P, or_else_helper<error_t, V> >;
};

template<auto F, typename P>
struct GetTransformer<OrElseF<F>, P> {
    using error_t = details::error_of_t<details::result_type_t<P>>;
    using type = Handle<P, or_elsef_helper<error_t, F> >;
};

}



}