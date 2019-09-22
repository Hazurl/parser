#pragma once

#include <type_traits>
#include <variant>
#include <experimental/type_traits>

#include <wpr/containers/Parser.hpp>
#include <wpr/details/List.hpp>

namespace wpr::details {

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
    using parser_type = typename P::parser_type;
    using result_type = typename P::result_type;
    static inline constexpr bool value{ true };
};

template<typename P, typename R>
static inline constexpr bool is_parser_v{ is_parser<P, R, void>::value };
template<typename P>
static inline constexpr bool is_parser_soft_check_v{ is_parser<P, soft_check_t, void>::value };
template<typename P>
using parsed_type_t = typename P::parsed_type;
template<typename P>
using parser_type_t = typename P::parser_type;
template<typename P>
using result_type_t = typename P::result_type;
template<typename P>
static inline constexpr bool can_fail_v{ length_v<list_from_errors_t<result_type_t<P>>> > 0 };






/*
    Construct a Parser-type from a list of errors
 */
template<typename P, typename T, typename L>
struct ErrorsToParser {};

template<typename P, typename T, typename...Es>
struct ErrorsToParser<P, T, List<Es...>> {
    using type = Parser<P, T, Es...>;
};

template<typename P, typename T, typename L>
using parser_from_list_t = typename ErrorsToParser<P, T, L>::type;





/*
    Validate a parser
 */
namespace parser_details {

template<typename...Ts>
struct TypePrinter {};

template<typename...>
struct False { static constexpr bool value{ false }; };

template<typename...Ts>
constexpr bool false_v = False<Ts...>::value;

}

template<typename Res, typename T, typename...Es>
struct ValidateResult {
    parser_details::TypePrinter<Res, T, Es...> _;
    static_assert(parser_details::false_v<T, Es...>, "The parse method's return type should be a Result");
    static constexpr bool value = false;
};

/* All good */
template<typename T, typename E>
struct ValidateResult<Result<T, E>, T, E> {
    parser_details::TypePrinter<T, E> _;
    static constexpr bool value = true;
};

template<typename RT, typename T, typename...Es>
struct ValidateResult<Result<RT, Es...>, T, Es...> {
    parser_details::TypePrinter<RT, T, Es...> _;
    static_assert(parser_details::false_v<T>, "The parse method's return type is a Result with the wrong success type");
    static constexpr bool value = false;
};

template<typename...REs, typename T, typename...Es>
struct ValidateResult<Result<T, REs...>, T, Es...> {
    parser_details::TypePrinter<REs..., T, Es...> _;
    static_assert(parser_details::false_v<Es...>, "The parse method's return type is a Result with the wrong error list");
    static constexpr bool value = false;
};

template<typename...REs, typename RT, typename T, typename...Es>
struct ValidateResult<Result<RT, REs...>, T, Es...> {
    parser_details::TypePrinter<RT, REs..., T, Es...> _;
    static_assert(parser_details::false_v<T, Es...>, "The parse method's return type is a Result with the wrong success type and error list");
    static constexpr bool value = false;
};

template<typename P, typename R, typename T, typename...Es> 
struct ValidateParseFunctionType {
    parser_details::TypePrinter<P, R, T, Es...> _;
    static_assert(parser_details::false_v<P>, "The parse member is detected but doesn't seems like a static method, makes sure it has the signature: \ntemplate<typename Reader>\nstatic Result<T, Es...> parse(Reader reader)");
    static constexpr bool value = false;
};

/* All good */
template<typename Res, typename R, typename T, typename...Es> 
struct ValidateParseFunctionType<Res(*)(R), R, T, Es...> : ValidateResult<Res, T, Es...> { parser_details::TypePrinter<Res, R, T, Es...> _; };

template<typename Res, typename...Args, typename R, typename T, typename...Es> 
struct ValidateParseFunctionType<Res(*)(Args...), R, T, Es...> : ValidateResult<Res, T, Es...> {
    parser_details::TypePrinter<Res, Args..., R, T, Es...> _;
    static_assert(parser_details::false_v<R>, "The parse method should only have one argument, the reader (taken by value)");
    static constexpr bool value = false;
};

template<typename Res, typename...Args, typename C, typename R, typename T, typename...Es> 
struct ValidateParseFunctionType<Res(C::*)(Args...), R, T, Es...> : ValidateResult<Res, T, Es...> {
    parser_details::TypePrinter<Res, Args..., R, T, Es...> _;
    static_assert(parser_details::false_v<R>, "The parse method should be static");
    static constexpr bool value = false;
};

template<typename, typename R, typename P, typename T, typename...Es>
struct ValidateParseFunction {
    parser_details::TypePrinter<R, P, T, Es...> _;
    static_assert(parser_details::false_v<R>, "The parse method isn't detected, makes sure it has the signature: \ntemplate<typename Reader>\nstatic Result<T, Es...> parse(Reader reader)");
    static constexpr bool value = false;
};

/* All good */
template<typename R, typename P, typename T, typename...Es>
struct ValidateParseFunction<std::void_t<decltype(&P::template parse<R>)>, R, P, T, Es...> : ValidateParseFunctionType<decltype(&P::template parse<R>), R, T, Es...> { parser_details::TypePrinter<R, P, T, Es...> _; };

template<typename R, typename P>
struct ValidateParser {
    parser_details::TypePrinter<R, P> _;
    static_assert(parser_details::false_v<P>, "The type you wants to validate doesn't inherit from Parser, to be more exact, the parser_type alias isn't a parser");
    static constexpr bool value = false;
};

/* All good */
template<typename R, typename P, typename T, typename...Es>
struct ValidateParser<R, Parser<P, T, Es...>> : ValidateParseFunction<void, R, P, T, Es...> { parser_details::TypePrinter<R, P, T, Es...> _; };

template<typename R, typename P, typename = void>
struct ValidateCustomParser {
    parser_details::TypePrinter<R, P> _;
    static_assert(parser_details::false_v<P>, "The type you wants to validate doesn't inherit from Parser");
    static constexpr bool value = false;
};

/* All good */
template<typename R, typename P>
struct ValidateCustomParser<R, P, std::enable_if_t<std::is_base_of_v<parser_type_t<P>, P>>> : ValidateParser<R, parser_type_t<P>> { parser_details::TypePrinter<R, P> _; };


template<typename R, typename P>
constexpr bool is_parser_valid_v = ValidateCustomParser<R, P, void>::value;

}

namespace wpr {
 
 
/*
    Parse from value
 */
template<typename P, typename R>
auto parse(P, R reader) {
    static_assert(is_parser_v<P>, "P is not a parser");
    return P::parse(std::move(reader));
}


}
