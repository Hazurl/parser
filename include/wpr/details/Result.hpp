#pragma once

#include <wpr/containers/Result.hpp>
#include <wpr/details/List.hpp>

#include <utility>
#include <tuple>

namespace wpr::details {

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





namespace wpr {

namespace {

/*
    I hope God will forget me for relying on internal functions...
    But I'm forced ! The compiler wasn't able to deduce the template arguments.
    `error: no matching function for call to ‘__get_helper<0>(std::tuple<...>&)`
    `candidate: ‘template<long unsigned int __i, class _Head, class ... _Tail> constexpr _Head& std::__get_helper(std::_Tuple_impl<_Idx, _Head, _Tail ...>&)`
    `argument ‘1’ does not match ‘0’`

    If at some point this code is responsible for a bug, blame the compiler not me, I'm just doing my job...

    -- Anonymous (because, obviously, I won't give my name)
*/

template<std::size_t I, typename...Args, typename...Tail>
decltype(auto) get(std::tuple<Args...>& tuple, details::List<Tail...>) {
    return std::move(std::__get_helper<I, Tail...>(tuple));
}

template<typename T, typename Tag, typename...Args, size_t...Is>
T make_from_tuple_impl(Tag tag, std::tuple<Args...>&& tuple, std::index_sequence<Is...>) {
    return T(tag, std::move(get<Is>(tuple, details::slice_t<Is, sizeof...(Args) - Is, details::List<Args...>>{}))...);
}

template<typename T, typename Tag, typename...Args>
T make_from_tuple(Tag tag, std::tuple<Args...>&& tuple) {
    return make_from_tuple_impl<T>(tag, std::move(tuple), std::make_index_sequence<sizeof...(Args)>{});
}

}

/*
    success function
    -- used to create a Result with any errors
 */
template<typename...Args>
decltype(auto) success(std::size_t cursor, Args&&...args) {
    return ResultBuilder{ [tuple = std::make_tuple(cursor, std::forward<Args>(args)...)] (auto r) mutable { 
        using T = typename decltype(r)::type;

        return make_from_tuple<T>(success_tag, std::move(tuple));

        // I secretly hope this code will work sometimes in the future
        /*return std::apply([] (auto&&... values) { 
            return T(success_tag, std::forward<decltype(values)>(values)...); 
        }, std::move(tuple));*/
    }};
}






/*
    fail function
    -- used to create a Result with any success and errors
 */
template<typename...Args>
decltype(auto) fail(std::size_t cursor, Args&&...args) {
    return ResultBuilder{ [tuple = std::make_tuple(cursor, std::forward<Args>(args)...)] (auto r) mutable { 
        using T = typename decltype(r)::type;

        return make_from_tuple<T>(error_tag, std::move(tuple));

        // I secretly hope this code will work sometimes in the future
        /*return std::apply([] (auto&&... values) { 
            return T(error_tag, std::forward<decltype(values)>(values)...); 
        }, std::move(tuple)); */
    }};
}

}