#pragma once

#include <wpr/Containers.hpp>
#include <wpr/Details.hpp>
#include <wpr/parsers/Errors.hpp>

#include <cassert>
#include <type_traits>

namespace wpr {

/*
    Unit parser
 */

template<auto V>
struct Unit : Parser<Unit<V>, std::remove_cv_t<decltype(V)>> {

    template<typename R>
    static Result<std::remove_cv_t<decltype(V)>> parse(R reader) {
        return success(reader.cursor(), V);
    }

};





/*
    Parser as value
 */

template<auto V>
constexpr Unit<V> unit;





/*
    UnitF parser
 */

template<auto& F>
struct UnitF : Parser<UnitF<F>, decltype(F())> {

    template<typename R>
    static Result<decltype(F())> parse(R reader) {
        return success(reader.cursor(), F());
    }

};





/*
    Parser as value
 */

template<auto& F>
constexpr UnitF<F> unitf;





}