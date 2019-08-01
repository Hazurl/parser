#pragma once

#include <ws/parser2/Containers.hpp>
#include <ws/parser2/Details.hpp>
#include <ws/parser2/parsers/Errors.hpp>

namespace ws::parser2 {

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
            return fail<error::EndOfFile>(reader.cursor());
        }

        return success(reader.cursor(), reader.peek());
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