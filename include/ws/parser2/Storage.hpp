#pragma once

#include <variant>
#include <tuple>
#include <optional>
#include <iostream>
#include <experimental/type_traits>

namespace ws::parser2 {

template<typename...Ts>
struct Product : std::tuple<Ts...> {};

namespace {

template<typename...Ts, std::size_t...Is>
std::ostream& print_product_helper(std::ostream& os, Product<Ts...> const& prod, std::index_sequence<Is...>) {
    return ((os << "(") << ... << std::get<Is>(prod)) << ")";
}

template<typename...Ts, std::size_t...Is>
bool equal_product_helper(Product<Ts...> const& lhs, Product<Ts...> const& rhs, std::index_sequence<Is...>) {
    return (... && (std::get<Is>(lhs) == std::get<Is>(rhs)));
}

}

template<typename...Ts>
std::ostream& operator <<(std::ostream& os, Product<Ts...> const& prod) {
    return print_product_helper(os, prod, std::make_index_sequence<sizeof...(Ts)>{});
}

template<typename...Ts>
bool operator ==(Product<Ts...> const& lhs, Product<Ts...> const& rhs) {
    return equal_product_helper(lhs, rhs, std::make_index_sequence<sizeof...(Ts)>{});
}

template<typename...Ts>
struct Sum : std::variant<Ts...> {};

template<typename...Ts>
std::ostream& operator <<(std::ostream& os, Sum<Ts...> const& sum) {
    os << "(#" << sum.index() << ": "; 
    std::visit([&os] (auto const& v) { os << v; }, sum); 
    return os << ")";
}

namespace {
    template<typename A, typename B>
    using decltype_equal_operator = decltype(std::declval<A>() == std::declval<B>());

    template<typename A, typename B>
    constexpr bool detect_equal_operator_v = std::experimental::is_detected_convertible_v<bool, decltype_equal_operator, A, B>;
}

template<typename...Ts>
bool operator ==(Sum<Ts...> const& lhs, Sum<Ts...> const& rhs) {
    return std::visit(
        [] (auto const& l, auto const& r) { 
            if constexpr (detect_equal_operator_v<std::decay_t<decltype(l)>, std::decay_t<decltype(r)>>) {
                return l == r; 
            } else {
                return false; 
            }
        }
    , lhs, rhs);
}

template<class... Ts> struct Visitor : Ts... { using Ts::operator()...; };
template<class... Ts> Visitor(Ts...) -> Visitor<Ts...>;

template<typename T>
struct Maybe : std::optional<T> {
    template<typename...Args>
    Maybe(Args&&...args) : std::optional<T>(std::forward<Args>(args)...) {}
};

template<typename T> Maybe(T) -> Maybe<T>;

template<typename T>
std::ostream& operator <<(std::ostream& os, Maybe<T> const& maybe) {
    if (maybe) {
        return os << "*" << maybe.value();
    }

    return os << "null";
}

template<typename T>
bool operator ==(Maybe<T> const& lhs, Maybe<T> const& rhs) {
    if (lhs && rhs) {
        return *lhs == *rhs;
    }
    return !(bool(lhs) ^ bool(rhs));
}

template<typename S>
struct Success { 
    template<typename...Args>
    Success(Args&&...args) : value(std::forward<Args>(args)...) {}
    
    S value;
};

template<typename L>
struct ResultBuilder {
    L build;
};

template<typename L> ResultBuilder(L) -> ResultBuilder<L>;

namespace {
    template<typename T>
    struct Type { using type = T; };
}

template<typename S, typename...Es>
struct Result {
    std::size_t cursor;
    std::variant<Success<S>, Es...> value;

    template<std::size_t I, typename...Args>
    Result(std::in_place_index_t<I> i, std::size_t cursor, Args&&...args)
        : cursor{ cursor }, value(i, std::forward<Args>(args)...) {}

    template<typename...Args>
    Result(std::in_place_index_t<0>, std::size_t cursor, Args&&...args)
        : cursor{ cursor }, value(Success<S>(std::forward<Args>(args)...)) {}

    template<typename L>
    Result(ResultBuilder<L> l) 
        : Result(l.build(Type<Result<S, Es...>>{})) {}

    static constexpr bool can_fail{ sizeof...(Es) > 0 };

    std::string what() const {
        return std::visit(Visitor{
            [] (Success<S> const&) { return std::string{}; },
            [] (auto const& e) { return e.what(); }
        }, value);
    }

    bool is_success() const {
        return std::holds_alternative<Success<S>>(value);
    }

    template<typename E = void>
    bool is_error() const {
        static_assert((std::is_same_v<E, Es> || ...) || std::is_same_v<E, void>, "Unknown template parameter, please use one of the possible error (or void for any)");
        if constexpr (std::is_same_v<E, void>) {
            return !is_success();
        } else {
            return std::holds_alternative<E>(value);
        }

    }

    auto& success() & {
        return std::get<Success<S>>(value).value;
    }

    auto const& success() const& {
        return std::get<Success<S>>(value).value;
    }

    auto&& success() && {
        return std::move(std::get<Success<S>>(value).value);
    }

    template<typename E>
    auto& error() & {
        return std::get<E>(value);
    }

    template<typename E>
    auto const& error() const& {
        return std::get<E>(value);
    }

    template<typename E>
    auto&& error() && {
        return std::move(std::get<E>(value));
    }

};


template<typename P, auto& F>
struct Transformer;

template<typename P, typename T, typename...E>
struct Parser {
    using parsed_type = T;
    using result_type = Result<T, E...>;

    template<typename F>
    Transformer<P, F::func> operator[] (F) const;
};


}