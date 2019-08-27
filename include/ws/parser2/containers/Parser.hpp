#pragma once

#include <ostream>

#include <ws/parser2/containers/Result.hpp>

namespace ws::parser2 {

template<typename P, auto& F>
struct Transformer;

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
    Transformer<P, F::func> operator[] (F) const;
};

}