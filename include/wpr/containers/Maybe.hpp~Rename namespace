#pragma once

#include <optional>
#include <ostream>

#include <wpr/Details.hpp>

namespace wpr {

/*
    Maybe type
 */

template<typename T>
struct Maybe : std::optional<T> {
    template<typename...Args>
    Maybe(Args&&...args) : std::optional<T>(std::forward<Args>(args)...) {}
};

template<typename T> Maybe(T) -> Maybe<T>;






/*
    Operator <<
 */

template<typename T>
std::ostream& operator <<(std::ostream& os, Maybe<T> const& maybe) {
    if (maybe) {
        return os << "*" << maybe.value();
    }

    return os << "null";
}






/*
    Operator ==
 */

template<typename T>
bool operator ==(Maybe<T> const& lhs, Maybe<T> const& rhs) {
    if (lhs && rhs) {
        return *lhs == *rhs;
    }
    return !(bool(lhs) ^ bool(rhs));
}




}