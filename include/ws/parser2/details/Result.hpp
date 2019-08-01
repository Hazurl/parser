#pragma once

#include <ws/parser2/containers/Result.hpp>
#include <ws/parser2/details/List.hpp>

namespace ws::parser2::details {

/*
    Set the result's sucess
*/

template<typename S, typename R>
struct SetSuccess {};

template<typename S, typename S_, typename...Es>
struct SetSuccess<S, Result<S_, Es...>> {
    using type = Result<S, Es...>;
};

template<typename S, typename R>
using set_success_t = typename SetSuccess<S, R>::type;

template<typename R>
struct SuccessOf {};

template<typename S, typename...Es>
struct SuccessOf<Result<S, Es...>> { 
    using type = S;
};

template<typename R>
using success_of_t = typename SuccessOf<R>::type;

}





namespace ws::parser2 {

/*
    success function
    -- used to create a Result with any errors
 */
template<typename...Args>
decltype(auto) success(std::size_t cursor, Args&&...args) {
    return ResultBuilder{ [&] (auto r) { 
        using T = typename decltype(r)::type;
        return T(std::in_place_index_t<0>{}, cursor, std::forward<Args>(args)...); 
    }};
}






/*
    fail function
    -- used to create a Result with any success and errors
 */
template<typename E, typename...Args>
decltype(auto) fail(std::size_t cursor, Args&&...args) {
    return ResultBuilder{ [&] (auto r) { 
        using T = typename decltype(r)::type;
        using L = details::list_from_errors_t<T>;
        static_assert(details::is_in_v<E, L>, "Failure type mismatch, error isn't in the result");
        /* index 0 is the success type, errors starts at 1 */
        static constexpr std::size_t error_index =  details::index_of_v<L, E> + 1;

        return T(std::in_place_index_t<error_index>{}, cursor, std::forward<Args>(args)...); 
    }};
}

}