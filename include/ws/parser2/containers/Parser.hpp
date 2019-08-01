#pragma once

#include <ostream>

#include <ws/parser2/containers/Result.hpp>

namespace ws::parser2 {

template<typename P, auto& F>
struct Transformer;

/*
    Product type
 */

template<typename P, typename T, typename...Es>
struct Parser {
    using parsed_type = T;
    using result_type = Result<T, Es...>;
    using parser_type = Parser<P, T, Es...>;

    static constexpr bool can_fail{ result_type::can_fail };

    template<typename F>
    Transformer<P, F::func> operator[] (F) const;
};

}