#pragma once

#include <type_traits>

#include <wpr/Parsers.hpp>

namespace wpr {

namespace {

template<typename E, auto V>
auto or_else_helper(E) {
    return V;
}

}

template<auto V>
struct OrElse {};

template<auto V>
constexpr OrElse<V> or_else;


namespace details {

template<auto V, typename P>
struct GetTransformer<OrElse<V>, P> {
    using error_t = details::error_of_t<details::result_type_t<P>>;
    using type = Handle<P, or_else_helper<error_t, V> >;
};

}



}