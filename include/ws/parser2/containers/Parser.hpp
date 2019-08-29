#pragma once

#include <ostream>

#include <ws/parser2/containers/ParserHeader.hpp>
#include <ws/parser2/parsers/Transformer.hpp>

namespace ws::parser2 {

template<typename P, typename T, typename E>
template<typename F>
Transformer<P, F::value> Parser<P, T, E>::operator[] (F) const {
    return Transformer<P, F::value>{};
}

}