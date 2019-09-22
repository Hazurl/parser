#pragma once

#include <type_traits>

#include <wpr/Parsers.hpp>

namespace wpr {

namespace {

template<typename T, typename E>
T panic_helper(E err) {
    throw err;
}

}

struct Panic {};
constexpr Panic panic;


namespace details {

template<typename P>
struct GetTransformer<Panic, P> {
    using error_t = details::error_of_t<details::result_type_t<P>>;
    using success_t = details::parsed_type_t<P>;
    using type = Handle<P, panic_helper<success_t, error_t> >;
};

}



}