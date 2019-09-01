#pragma once

#include <string>
#include <ostream>

#include <ws/parser2/details/Describe.hpp>

namespace ws::parser2::error {

/*
    EnfOfFile type
    -- represents an unexpected reach of the end of the file
 */
struct PredicateFailure {
    std::string what() const {
        return "The predicate hasn't been satisfied";
    }
};




/*
    Operator <<
 */

std::ostream& operator <<(std::ostream& os, PredicateFailure) {
    return os << "The predicate hasn't been satisfied";
}





/*
    Operator ==
 */

bool operator ==(PredicateFailure, PredicateFailure) {
    return true;
}

}

namespace ws::parser2 {





/*
    Describe
 */

template<>
struct Describe<error::PredicateFailure> {
    std::string operator()(error::PredicateFailure) {
        return "The predicate hasn't been satisfied";
    }
};




}