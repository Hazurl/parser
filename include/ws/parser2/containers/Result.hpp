#pragma once

#include <ostream>

#include <ws/parser2/details/Visitor.hpp>

namespace ws::parser2 {

/*
    Success type
    -- used to differentiate errors from the success type
 */

template<typename S>
struct Success { 
    template<typename...Args>
    Success(Args&&...args) : value(std::forward<Args>(args)...) {}
    
    S value;
};






/*
    ResultBuilder type
    -- used to create any Result
 */

template<typename L>
struct ResultBuilder {
    L build;
};

template<typename L> ResultBuilder(L) -> ResultBuilder<L>;





/*
    Type helper type
    -- used to tell the ResultBuilder's lambda which Result to create
 */

namespace  {
    template<typename T>
    struct Type { using type = T; };
}






/*
    Result type
 */

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
        return std::visit(details::Visitor{
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






}