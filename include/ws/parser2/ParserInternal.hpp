#pragma once

#include <variant>
#include <tuple>

#include <module/module.h>

namespace ws::parser2 {

template<typename S>
struct Success { 
    S value;
    std::size_t cursor;
};

template<typename S, typename...Es>
struct Result {
    std::variant<Success<S>, Es...> value;

    static inline constexpr bool can_fail{ sizeof...(Es) > 0 };

    bool is_success() const {
        return std::holds_alternative<Success<S>>(value);
    }

    bool is_error() const {
        return !is_success();
    }

    template<typename R>
    bool is_specific_error() const {
        return std::holds_alternative<R>(value);
    }

    auto& success() & {
        return std::get<Success<S>>(value);
    }

    auto const& success() const& {
        return std::get<Success<S>>(value);
    }

    auto&& success() && {
        return std::move(std::get<Success<S>>(value));
    }

    template<typename R>
    auto& error() & {
        return std::get<R>(value);
    }

    template<typename R>
    auto const& error() const& {
        return std::get<R>(value);
    }

    template<typename R>
    auto&& error() && {
        return std::move(std::get<R>(value));
    }

};

// Parser :: Input -> Success | Error
template<typename S, typename...Es>
struct Parser {
    using result_t = Result<S, Es...>;
    using parser_t = Parser<S, Es...>;
    using success_t = Success<S>;
    using success_arg_t = S;

    static inline constexpr bool is_parser{ true };
    static inline constexpr bool can_fail{ result_t::can_fail };
};


struct EndOfFile {};

// Consume the next character in the input
// If it's empty, returns EndOfFile
template<typename C = char>
struct GetC : Parser<C, EndOfFile> {
    using typename Parser<C, EndOfFile>::result_t;
    using typename Parser<C, EndOfFile>::parser_t;
    using typename Parser<C, EndOfFile>::success_t;

    template<typename In, typename Ctx>
    result_t parse(In const& in, Ctx&&) const {
        if (in.empty()) {
            return result_t{ EndOfFile{} };
        }

        return result_t{
            success_t {
                in.get(),
                in.cursor + 1
            }
        };
    }
};

using Get = GetC<char>;

template<class... Ts> struct Visitor : Ts... { using Ts::operator()...; };
template<class... Ts> Visitor(Ts...) -> Visitor<Ts...>;

template<typename S, typename R>
struct MapSuccess {};

template<typename S, typename S_, typename...Es>
struct MapSuccess<S, Result<S_, Es...>> {
    using type = Result<S, Es...>;
};

template<typename S, typename R>
using map_success_t = typename MapSuccess<S, R>::type;

template<typename T, typename R>
struct PushFrontTuple {};

template<typename T, typename...Ts>
struct PushFrontTuple<T, std::tuple<Ts...>> {
    using type = std::tuple<T, Ts...>;
};

template<typename T, typename R>
using puhs_front_tuple_t = typename PushFrontTuple<T, R>::type;

template<std::size_t B, std::size_t C, typename T>
struct TupleSlice {};

template<std::size_t B, std::size_t C>
struct TupleSlice<B, C, std::tuple<>> {
    using type = std::tuple<>;
};

template<typename T, typename...Ts>
struct TupleSlice<0, 0, std::tuple<T, Ts...>> {
    using type = std::tuple<T>;
};

template<std::size_t C, typename T, typename...Ts>
struct TupleSlice<0, C, std::tuple<T, Ts...>> {
    using type = puhs_front_tuple_t<T, typename TupleSlice<0, C-1, std::tuple<Ts...>>::type>;
};

template<std::size_t B, std::size_t C, typename T, typename...Ts>
struct TupleSlice<B, C, std::tuple<T, Ts...>> : TupleSlice<B-1, C, std::tuple<Ts...>> {};

template<std::size_t B, std::size_t C, typename T>
using tuple_slice_t = typename TupleSlice<B, C, T>::type;

template<typename T, typename E, typename R, typename...Rs>
struct IfIn {};

template<typename T, typename E, typename R>
struct IfIn<T, E, R> {
    using type = E;
};

template<typename T, typename E, typename R, typename...Rs>
struct IfIn<T, E, R, R, Rs...> {
    using type = T;
};

template<typename T, typename E, typename R, typename R0, typename...Rs>
struct IfIn<T, E, R, R0, Rs...> : IfIn<T, E, R, Rs...> {};

template<typename P, typename E>
struct PushError {};

template<typename S, typename...Es, typename E>
struct PushError<Parser<S, Es...>, E> {
    using type = Parser<S, E, Es...>;
};

template<typename P>
struct RemoveErrorDuplicate {};

template<typename S>
struct RemoveErrorDuplicate<Parser<S>> {
    using type = Parser<S>;
};

template<typename S, typename E, typename...Es>
struct RemoveErrorDuplicate<Parser<S, E, Es...>> {
    using type = typename IfIn<typename RemoveErrorDuplicate<Parser<S, Es...>>::type, typename PushError<typename RemoveErrorDuplicate<Parser<S, Es...>>::type, E>::type, E, Es...>::type;
};

template<typename S, typename...Ps>
struct AccumulateErrors {};

template<typename SA, typename...EAs>
struct AccumulateErrors<Parser<SA, EAs...>> {
    using type = typename RemoveErrorDuplicate<Parser<SA, EAs...>>::type;
};

template<typename SA, typename...Es, typename SB, typename...Rs, typename...Ps>
struct AccumulateErrors<Parser<SA, Es...>, Parser<SB, Rs...>, Ps...>
    : AccumulateErrors<Parser<SA, Es..., Rs...>, Ps...> {};

template<typename S, typename...Ps>
using AccumulateErrors_t = typename AccumulateErrors<Parser<S>, Ps...>::type;

// Product :: [Parser<S, Es...>] -> Parser<[S], Es...>
template<typename...Ps>
struct Product;

template<class...Ps> Product(Ps...) -> Product<Ps...>;

template<typename P, typename...Ps>
struct Product<P, Ps...> : AccumulateErrors_t<std::tuple<typename P::success_arg_t, typename Ps::success_arg_t...>, typename P::parser_t, typename Ps::parser_t...> {
    using typename AccumulateErrors_t<std::tuple<typename P::success_arg_t, typename Ps::success_arg_t...>, typename P::parser_t, typename Ps::parser_t...>::result_t;
    using typename AccumulateErrors_t<std::tuple<typename P::success_arg_t, typename Ps::success_arg_t...>, typename P::parser_t, typename Ps::parser_t...>::parser_t;
    using typename AccumulateErrors_t<std::tuple<typename P::success_arg_t, typename Ps::success_arg_t...>, typename P::parser_t, typename Ps::parser_t...>::success_t;

    Product(P p, Ps... ps) : p{ p }, ps{ ps... } {}

    P p;
    Product<Ps...> ps;
    template<typename In, typename Ctx>
    result_t parse(In const& in, Ctx&& ctx) const {
        auto value = p.parse(in, std::forward<Ctx>(ctx));
        return result_t{ 
            std::visit(Visitor{
                [&] (typename P::success_t&& success) {
                    auto new_input = in.copy_with_cursor(value.success().cursor);
                    auto next = ps.parse(new_input, std::forward<Ctx>(ctx));

                    return result_t{ 
                        std::visit(Visitor{
                            [&] (typename Product<Ps...>::success_t&& next_success) {
                                return result_t{ success_t{ std::tuple_cat(std::make_tuple(std::move(success.value), std::move(next_success.value))), next_success.cursor } };
                            },
                            [] (auto&& e) { 
                                return result_t{ std::move(e) }; 
                            }
                        }, std::move(next.value)) 
                    };
                },
                [] (auto&& e) { 
                    return result_t{ std::move(e) }; 
                }
            }, std::move(value.value))
        };
    }

    std::tuple<Ps...> parsers;

};

template<typename P>
struct Product<P> : P {};

template<>
struct Product<> : Parser<std::tuple<>> {
    using typename Parser<std::tuple<>>::result_t;
    using typename Parser<std::tuple<>>::parser_t;
    using typename Parser<std::tuple<>>::success_t;

    template<typename In, typename Ctx>
    result_t parse(In const&, Ctx&&) const {
        return result_t{ std::make_tuple() };
    }
};

template<typename...Ps>
struct Sum {

};

}