#pragma once

#include <wpr/Containers.hpp>
#include <wpr/Details.hpp>
#include <wpr/parsers/Errors.hpp>

namespace wpr {

/*
    Inverse parser
    -- if `P` work, returns the success as an error 
    -- if `P` fail, returns the error as a success 
 */

template<typename P>
struct Inverse : Parser<Inverse<P>, details::error_of_t<details::result_type_t<P>>, details::parsed_type_t<P> > {
    template<typename R>
    static details::result_type_t<Inverse<P>> parse(R reader) {
        auto res = P::parse(reader);

        if (res.is_error()) {
            return success(res.cursor(), std::move(res.error()));
        }

        return fail(res.cursor(), std::move(res.success()));
    }

};






/*
    Parser as value
 */

template<auto p>
constexpr Inverse<std::decay_t<decltype(p)>> inverse;

}