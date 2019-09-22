#pragma once

#include <string>
#include <ostream>

#include <wpr/details/Describe.hpp>

namespace wpr::error {

/*
    Expected type
    -- represents something expected
    -- for example
       decltype(some<nextc>)::parse(Reader{ "" })
       => Error: Expected 
 */
struct Expected {
    std::string what() const {
        return "Expected something";
    }
};




/*
    Operator <<
 */

std::ostream& operator <<(std::ostream& os, Expected) {
    return os << "Expected something";
}





/*
    Operator ==
 */

bool operator ==(Expected, Expected) {
    return true;
}

}

namespace wpr {





/*
    Describe
 */

template<>
struct Describe<error::Expected> {
    std::string operator()(error::Expected const&) {
        return "Expected something";
    }
};




}