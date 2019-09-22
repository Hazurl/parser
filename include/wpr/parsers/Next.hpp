#pragma once

#include <wpr/Containers.hpp>
#include <wpr/Details.hpp>
#include <wpr/parsers/Errors.hpp>

#include <cassert>

namespace wpr {

/*
    Next parser
    -- `C` is the type of the reader's tokens
    -- either returns the next token or the EOF error
 */

template<typename C>
struct Next : Parser<Next<C>, C, error::EndOfFile> {

    template<typename R>
    static Result<C, error::EndOfFile> parse(R reader) {
        if (reader.is_end()) {
            return fail(reader.cursor());
        }

        return success(reader.cursor() + 1, reader.peek());
    }

};





/*
    Parser as value
 */

template<typename C>
constexpr Next<C> next;






/*
    Alias for char
 */

using NextC = Next<char>;
constexpr NextC nextc;

}