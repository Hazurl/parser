#pragma once

#include <string>
#include <ostream>

#include <ws/parser2/details/Describe.hpp>

namespace ws::parser2::error {

/*
    Expected type
    -- represents somethign expected
    -- for example
       decltype(some<nextc>)::parse(Reader{ "" })
       => Error: Expected Next<char>
 */
template<typename P>
struct Expected {
    std::string what() const {
        return "Expected to parser P";
    }
};




/*
    Operator <<
 */

template<typename P>
std::ostream& operator <<(std::ostream& os, Expected<P>) {
    return os << "Expected to parser P";
}





/*
    Operator ==
 */

template<typename P>
bool operator ==(Expected<P>, Expected<P>) {
    return true;
}

}

namespace ws::parser2 {





/*
    Describe
 */

template<typename P>
struct Describe<error::Expected<P>> {
    std::string operator()(error::Expected<P> const&) {
        return "Expected to parser P";
    }
};




}