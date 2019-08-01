#pragma once

#include <ostream>
#include <variant>

namespace ws::parser2 {

/*
    Sum type
 */

template<typename...Ts>
struct Sum : std::variant<Ts...> {
    Sum() = default;
    Sum(Ts... ts) : std::variant<Ts...>(std::move(ts)...) {};
};

template<typename...Ts> Sum(Ts...) -> Sum<Ts...>;






/*
    Operator <<
 */

template<typename...Ts>
std::ostream& operator <<(std::ostream& os, Sum<Ts...> const& sum) {
    os << "(#" << sum.index() << ": "; 
    std::visit([&os] (auto const& v) { os << v; }, sum); 
    return os << ")";
}






/*
    Operator ==
 */

namespace sum_details {
    template<typename A, typename B>
    using decltype_equal_operator = decltype(std::declval<A>() == std::declval<B>());

    template<typename A, typename B>
    constexpr bool detect_equal_operator_v = std::experimental::is_detected_convertible_v<bool, decltype_equal_operator, A, B>;
}

template<typename...Ts>
bool operator ==(Sum<Ts...> const& lhs, Sum<Ts...> const& rhs) {
    return std::visit(
        [] (auto const& l, auto const& r) { 
            if constexpr (sum_details::detect_equal_operator_v<std::decay_t<decltype(l)>, std::decay_t<decltype(r)>>) {
                return l == r; 
            } else {
                return false; 
            }
        }
    , lhs, rhs);
}

}