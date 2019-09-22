#pragma once

#include <ostream>
#include <variant>

#include <wpr/Details.hpp>

namespace wpr {


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
    template trickery to makes SFINAE works
 */

namespace  {
    template<bool B, typename...>
    inline constexpr bool sfinae_ignore_v{ B };
}






/*
    Is E an error ?
 */

namespace  {
    template<typename E>
    inline constexpr bool can_fail_with{ !std::is_same_v<E, void> };
}






/*
    In place type
    -- disambiguation tags taht can be passed to Result's constructor
 */

using success_tag_t = std::in_place_index_t<0>;
using error_tag_t = std::in_place_index_t<1>;

inline constexpr success_tag_t success_tag{};
inline constexpr error_tag_t error_tag{};








/*
    Result type
 */

template<typename S, typename E = void>
struct Result {

    static constexpr bool can_fail{ can_fail_with<E> };

private:

    using container_t = std::conditional_t<can_fail, std::variant<S, E>, S>;

    static inline constexpr std::size_t success_index{ 0 };
    static inline constexpr std::size_t error_index{ 1 };

    std::size_t cursor_index{ 0 };
    container_t value;

public:

    template<typename...Args>
    Result(std::enable_if_t<sfinae_ignore_v<can_fail, S, Args...>, error_tag_t> tag, std::size_t cursor, Args&&...args)
        : cursor_index{ cursor }, value(tag, std::forward<Args>(args)...) {}

    template<typename...Args>
    Result(std::enable_if_t<sfinae_ignore_v<can_fail, S, Args...>, success_tag_t> tag, std::size_t cursor, Args&&...args)
        : cursor_index{ cursor }, value(tag, std::forward<Args>(args)...) {}

    template<typename...Args>
    Result(std::enable_if_t<!sfinae_ignore_v<can_fail, S, Args...>, success_tag_t>, std::size_t cursor, Args&&...args)
        : cursor_index{ cursor }, value(std::forward<Args>(args)...) {}

    template<typename L>
    Result(ResultBuilder<L> l) 
        : Result(l.build(Type<Result<S, E>>{})) {}

    bool is_success() const {
        if constexpr(can_fail) {
            return value.index() == success_index;
        } else {
            return true;
        }
    }

    bool is_error() const {
        if constexpr (can_fail) {
            return value.index() == error_index;
        } else {
            return false;
        }

    }

    template<typename T = E>
    std::enable_if_t<can_fail_with<T> && std::is_same_v<T, E>, 
    T const&> error() const& {
        return std::get<error_index>(value);
    }

    template<typename T = E>
    std::enable_if_t<can_fail_with<T> && std::is_same_v<T, E>, 
    T&> error() & {
        return std::get<error_index>(value);
    }

    template<typename T = E>
    std::enable_if_t<can_fail_with<T> && std::is_same_v<T, E>, 
    T&&> error() && {
        return std::get<error_index>(std::move(value));
    }

    S const& success() const& {
        if constexpr(can_fail) {
            return std::get<success_index>(value);
        } else {
            return value;
        }
    }

    S& success() & {
        if constexpr(can_fail) {
            return std::get<success_index>(value);
        } else {
            return value;
        }
    }

    S&& success() && {
        if constexpr(can_fail) {
            return std::get<success_index>(std::move(value));
        } else {
            return std::move(value);
        }
    }

    std::size_t cursor() const {
        return cursor_index;
    }

};




/*
    Describe
 */
template<typename S, typename E>
struct Describe<Result<S, E>> {
    std::string operator()(Result<S, E> const& res) {
        if constexpr(Result<S, E>::can_fail) {
            if (res.is_error()) {
                return describe(res.error());
            }
        }

        return describe(res.success());
    }
};




}