#pragma once

#include <type_traits>
#include <experimental/type_traits>

#include <ws/parser2/containers/Result.hpp>

namespace ws::parser2::details {

template<typename...Ts>
struct List;





/*
    Push an element `T` to a list `L`
*/
template<typename T, typename L>
struct Push {};

template<typename T, typename...Ts>
struct Push<T, List<Ts...>> {
    using type = List<T, Ts...>;
};

template<typename T, typename L>
using push_t = typename Push<T, L>::type;




/*
    Select `C` element starting from `B` in the list `L`
*/
template<std::size_t B, std::size_t C, typename L>
struct Slice {};

template<std::size_t B, std::size_t C>
struct Slice<B, C, List<>> {
    using type = List<>;
};

template<typename T, typename...Ts>
struct Slice<0, 0, List<T, Ts...>> {
    using type = List<T>;
};

template<std::size_t C, typename T, typename...Ts>
struct Slice<0, C, List<T, Ts...>> {
    using type = push_t<T, typename Slice<0, C-1, List<Ts...>>::type>;
};

template<std::size_t B, std::size_t C, typename T, typename...Ts>
struct Slice<B, C, List<T, Ts...>> : Slice<B-1, C, List<Ts...>> {};

template<std::size_t B, std::size_t C, typename L>
using slice_t = typename Slice<B, C, L>::type;





/*
    Returns whether the element `T` is in the list `L`
*/
template<typename T, typename L>
struct IsIn {};

template<typename T>
struct IsIn<T, List<>> : std::false_type {};

template<typename T, typename...Ts>
struct IsIn<T, List<T, Ts...>> : std::true_type {};

template<typename T, typename _, typename...Ts>
struct IsIn<T, List<_, Ts...>> : IsIn<T, List<Ts...>> {};

template<typename T, typename L>
inline constexpr bool is_in_v = IsIn<T, L>::value; 





/*
    Push an element `T` to a list `L` if it's not present
*/
template<typename T, typename L>
using push_unique_t = std::conditional_t<is_in_v<T, L>, L, push_t<T, L>>;






/*
    Concatenate the lists `A` and `B`
*/
template<template<typename, typename>typename P, typename A, typename B>
struct ConcatenateWith {};

template<template<typename, typename>typename P, typename B>
struct ConcatenateWith<P, List<>, B> {
    using type = B;
};

template<template<typename, typename>typename P, typename A, typename...As, typename B>
struct ConcatenateWith<P, List<A, As...>, B> {
    using type = P<A, typename ConcatenateWith<P, List<As...>, B>::type>;
};

template<typename A, typename B>
using concatenate_t = typename ConcatenateWith<push_t, A, B>::type; 

template<typename A, typename B>
using concatenate_unique_t = typename ConcatenateWith<push_unique_t, A, B>::type; 







/*
    List conversion
*/
template<typename L, template<typename...>typename M>
struct ListTo {};

template<typename...Ts, template<typename...>typename M>
struct ListTo<List<Ts...>, M> {
    using type = M<Ts...>;
};

template<template<typename...>typename M, typename N>
struct ListFrom {};

template<template<typename...>typename M, typename...Ts>
struct ListFrom<M, M<Ts...>> {
    using type = List<Ts...>;
};

template<typename R>
struct ErrorToList {};

template<typename S, typename...Es>
struct ErrorToList<Result<S, Es...>> {
    using type = List<Es...>;
};

template<typename L, template<typename...>typename M>
using list_to_t = typename ListTo<L, M>::type;

template<template<typename...>typename M, typename N>
using list_from_t = typename ListFrom<M, N>::type;

template<typename R>
using list_from_errors_t = typename ErrorToList<R>::type;





/*
    Index of element `E` in list `L`
*/
template<typename L, typename E, std::size_t I>
struct IndexOf {};

template<typename E, std::size_t I>
struct IndexOf<List<>, E, I> {};

template<typename...Es, typename E, std::size_t I>
struct IndexOf<List<E, Es...>, E, I> {
    static inline constexpr std::size_t value = I; 
};

template<typename _, typename...Es, typename E, std::size_t I>
struct IndexOf<List<_, Es...>, E, I> : IndexOf<List<Es...>, E, I-1> {};

template<typename L, typename E>
static inline constexpr std::size_t index_of_v{ IndexOf<L, E, 0>::value };

}