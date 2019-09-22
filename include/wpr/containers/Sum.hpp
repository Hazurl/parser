#pragma once

#include <ostream>
#include <variant>

#include<wpr/details/Describe.hpp>

namespace wpr {

/*
    Sum type
 */

struct from_variant_t {};

template<typename...Ts>
struct Sum : std::variant<Ts...> {
    //Sum() = default;
    template<typename...Args>
    Sum(Args&&...args) : std::variant<Ts...>(std::forward<Args>(args)...) {};
    Sum(from_variant_t, std::variant<Ts...> v) : std::variant<Ts...>(v) {}
};

template<typename...Ts> Sum(Ts...) -> Sum<Ts...>;
template<typename...Ts> Sum(from_variant_t, std::variant<Ts...>) -> Sum<Ts...>;






/*
    Operator <<
 */

template<typename...Ts>
std::ostream& operator <<(std::ostream& os, Sum<Ts...> const& sum) {
    os << "[#" << sum.index() << ": "; 
    std::visit([&os] (auto const& v) { os << v; }, static_cast<std::variant<Ts...> const&>(sum)); 
    return os << "]";
}






/*
    Describe
 */

template<typename...Ts>
struct Describe<Sum<Ts...>> {
    std::string operator()(Sum<Ts...> const& sum) {
        auto const& variant = static_cast<std::variant<Ts...> const&>(sum);
        return "[#" + std::to_string(variant.index()) + ": " + std::visit([] (auto const& v) { return describe(v); }, variant) + "]";
    }
};





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
    , static_cast<std::variant<Ts...> const&>(lhs), static_cast<std::variant<Ts...> const&>(rhs));
}

}