#pragma once

#include <string>
#include <type_traits>

namespace wpr {

template<typename T, typename = void>
struct Describe {
    std::string operator()(T const&) {
        return "~~ `describe` unimplemented for this type ~~";
    }
};

template<typename T>
std::string describe(T const& t) {
    return Describe<T>{}(t);
}

template<>
struct Describe<std::string> {
    std::string operator()(std::string const& s) {
        return s;
    }
};

template<typename T>
struct Describe<T, std::void_t<
    decltype(std::begin(std::declval<T>())),
    decltype(std::end(std::declval<T>()))
>> {
    std::string operator()(T const& t) {
        std::string str{ "[" };
        bool is_first{ true };
        for(auto const& v : t) {
            if(!is_first) {
                str += ", ";
            }

            is_first = false;

            str += describe(v);
        }

        return str + "]";
    }
};

template<>
struct Describe<char> {
    std::string operator()(char t) {
        return std::string{ "'" } + t + "' #" + std::to_string(static_cast<int>(t));
    }
};

template<typename T>
struct Describe<T, std::void_t<
    decltype(std::to_string(std::declval<T>()))
>> {
    std::string operator()(T const& t) {
        return std::to_string(t);
    }
};


}