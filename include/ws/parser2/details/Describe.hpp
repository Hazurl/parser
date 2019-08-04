#pragma once

#include <string>

namespace ws::parser2 {

template<typename T>
struct Describe {
    std::string operator()(T const&) {
        return "~~ `describe` unimplemented for this type ~~";
    }
};

template<typename T>
std::string describe(T const& t) {
    return Describe<T>{}(t);
}

}