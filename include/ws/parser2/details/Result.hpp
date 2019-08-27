#pragma once

#include <ws/parser2/containers/Result.hpp>
#include <ws/parser2/details/List.hpp>

namespace ws::parser2::details {

/*
    Set the result's sucess
*/

template<typename S, typename R>
struct SetSuccess {};

template<typename S, typename S_, typename E>
struct SetSuccess<S, Result<S_, E>> {
    using type = Result<S, E>;
};

template<typename S, typename R>
using set_success_t = typename SetSuccess<S, R>::type;

template<typename R>
struct SuccessOf {};

template<typename S, typename E>
struct SuccessOf<Result<S, E>> { 
    using type = S;
};

template<typename R>
using success_of_t = typename SuccessOf<R>::type;

template<typename R>
struct ErrorOf {};

template<typename S, typename E>
struct ErrorOf<Result<S, E>> { 
    using type = E;
};

template<typename R>
using error_of_t = typename ErrorOf<R>::type;


}





namespace ws::parser2 {

/*
    success function
    -- used to create a Result with any errors
 */
template<typename...Args>
decltype(auto) success(std::size_t cursor, Args&&...args) {
    return ResultBuilder{ [args = std::make_tuple(cursor, std::forward<Args>(args)...)] (auto r) { 
        using T = typename decltype(r)::type;
        return std::apply([] (auto&&... args) { 
            return T(success_tag, std::forward<decltype(args)>(args)...); 
        }, args); 
    }};
}






/*
    fail function
    -- used to create a Result with any success and errors
 */
template<typename...Args>
decltype(auto) fail(std::size_t cursor, Args&&...args) {
    return ResultBuilder{ [args = std::make_tuple(cursor, std::forward<Args>(args)...)] (auto r) { 
        using T = typename decltype(r)::type;

        return std::apply([] (auto&&... args) { 
            return T(error_tag, std::forward<decltype(args)>(args)...); 
        }, args); 
    }};
}

}