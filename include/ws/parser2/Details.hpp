#pragma once

#include <ws/parser2/Storage.hpp>

#include <type_traits>
#include <experimental/type_traits>

namespace ws::parser2::details {

template<typename...Ts>
struct List;

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
    Predicate to verify it's a parser
*/
namespace {
    struct soft_check_t {};

    template<bool b, typename P, typename R>
    struct LazyOrDetect {
        template<typename W>
        using detect_parser_t = decltype(W::template parse<R>);

        static inline constexpr bool value{ std::experimental::is_detected_exact_v<typename P::result_type(R), detect_parser_t, P> };
    };

    template<typename P, typename R>
    struct LazyOrDetect<true, P, R> {
        static inline constexpr bool value{ true };
    };

    template<bool b, typename P, typename R>
    static inline constexpr bool lazy_detect_or_v{ LazyOrDetect<b, P, R>::value };

}

template<typename P, typename R, typename>
struct is_parser {
    static inline constexpr bool value{ false };
};

template<typename P, typename R>
struct is_parser<P, R, std::void_t<typename P::parsed_type, typename P::result_type>> {
private:

public:

    static_assert(
        lazy_detect_or_v<std::is_same_v<R, soft_check_t>, P, R>,
        "Static method `parse` is not detected, it must be of type `template<typename R> Result<T, E...> parse(R)` where T is the parsed_type and E the possible errors"
    );

    using parsed_type = typename P::parsed_type;
    using result_type = typename P::result_type;
    static inline constexpr bool value{ true };
    static inline constexpr bool can_fail{ result_type::can_fail };
};

template<typename P, typename R>
static inline constexpr bool is_parser_v{ is_parser<P, R, void>::value };
template<typename P>
static inline constexpr bool is_parser_soft_check_v{ is_parser<P, soft_check_t, void>::value };
template<typename P>
static inline constexpr bool can_fail_v{ is_parser<P, soft_check_t, void>::can_fail };
template<typename P>
using parsed_type_t = typename is_parser<P, soft_check_t, void>::parsed_type;
template<typename P>
using result_type_t = typename is_parser<P, soft_check_t, void>::result_type;







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