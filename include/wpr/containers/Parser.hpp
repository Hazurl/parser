#pragma once

#include <ostream>

namespace wpr {

namespace details {

template<typename F, typename P>
struct GetTransformer {};

template<typename F, typename P>
using get_transformer_t = typename GetTransformer<F, P>::type;

}





/*
    Product type
 */

template<typename P, typename T, typename E = void>
struct Parser {
    using parsed_type = T;
    using result_type = Result<T, E>;
    using parser_type = Parser<P, T, E>;

    static constexpr bool can_fail{ result_type::can_fail };

    template<typename F>
    constexpr details::get_transformer_t<F, P> operator[] (F) const {
        return details::get_transformer_t<F, P>{};
    }
};

}