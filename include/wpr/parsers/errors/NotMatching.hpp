#pragma once

#include <string>
#include <ostream>

#include <wpr/details/Describe.hpp>

namespace wpr::error {

/*
    NotMatching error
    -- When a character was expected
 */
template<auto V>
struct NotMatching {
    std::string what() const {
        return "Was expecting " + describe(V);
    }
};




/*
    Operator <<
 */

template<auto V>
std::ostream& operator <<(std::ostream& os, NotMatching<V> e) {
    return os << e.what();
}





/*
    Operator ==
 */

template<auto V>
bool operator ==(NotMatching<V>, NotMatching<V>) {
    return true;
}

}

namespace wpr {





/*
    Describe
 */

template<auto V>
struct Describe<error::NotMatching<V>> {
    std::string operator()(error::NotMatching<V> const& e) {
        return e.what();
    }
};




}