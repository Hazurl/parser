#pragma once

#include <string>
#include <ostream>

#include <wpr/details/Describe.hpp>

namespace wpr::error {

/*
    NoBranch error
    -- When no branches's predicate succeed with the given input I
 */
template<typename I>
struct NoBranch {
    I value;

    std::string what() const {
        return "No branch matches " + describe(value);
    }
};




/*
    Operator <<
 */

template<typename I>
std::ostream& operator <<(std::ostream& os, NoBranch<I> const& e) {
    return os << e.what();
}





/*
    Operator ==
 */

template<typename I>
bool operator ==(NoBranch<I> const& lhs, NoBranch<I> const& rhs) {
    return lhs.value == rhs.value;
}

}

namespace wpr {





/*
    Describe
 */

template<typename I>
struct Describe<error::NoBranch<I>> {
    std::string operator()(error::NoBranch<I> const& e) {
        return e.what();
    }
};




}