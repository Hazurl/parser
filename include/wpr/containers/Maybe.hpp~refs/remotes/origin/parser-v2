#pragma once

#include <optional>
#include <ostream>

<<<<<<< refs/remotes/origin/parser-v2:include/wpr/containers/Maybe.hpp
namespace wpr {
=======
#include <ws/parser2/Details.hpp>

namespace ws::parser2 {
>>>>>>> Add some include:include/ws/parser2/containers/Maybe.hpp

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