#pragma once

#include <type_traits>
#include <experimental/type_traits>

namespace ws::parser2::details {

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


}